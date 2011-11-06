/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include <xmloff/families.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/nmspmap.hxx>
#include <tools/string.hxx>
#include <tools/debug.hxx>

#include "sheetdata.hxx"

// -----------------------------------------------------------------------

ScSheetSaveData::ScSheetSaveData() :
    mnStartTab( -1 ),
    mnStartOffset( -1 ),
    maPreviousNote( rtl::OUString(), rtl::OUString(), ScAddress(ScAddress::INITIALIZE_INVALID) ),
    mbInSupportedSave( false )
{
}

ScSheetSaveData::~ScSheetSaveData()
{
}

void ScSheetSaveData::AddCellStyle( const rtl::OUString& rName, const ScAddress& rCellPos )
{
    maCellStyles.push_back( ScCellStyleEntry( rName, rCellPos ) );
}

void ScSheetSaveData::AddColumnStyle( const rtl::OUString& rName, const ScAddress& rCellPos )
{
    maColumnStyles.push_back( ScCellStyleEntry( rName, rCellPos ) );
}

void ScSheetSaveData::AddRowStyle( const rtl::OUString& rName, const ScAddress& rCellPos )
{
    maRowStyles.push_back( ScCellStyleEntry( rName, rCellPos ) );
}

void ScSheetSaveData::AddTableStyle( const rtl::OUString& rName, const ScAddress& rCellPos )
{
    maTableStyles.push_back( ScCellStyleEntry( rName, rCellPos ) );
}

void ScSheetSaveData::HandleNoteStyles( const rtl::OUString& rStyleName, const rtl::OUString& rTextName, const ScAddress& rCellPos )
{
    // only consecutive duplicates (most common case) are filtered out here,
    // the others are found when the styles are created

    if ( rStyleName == maPreviousNote.maStyleName &&
         rTextName  == maPreviousNote.maTextStyle &&
         rCellPos.Tab() == maPreviousNote.maCellPos.Tab() )
    {
        // already stored for the same sheet - ignore
        return;
    }

    ScNoteStyleEntry aNewEntry( rStyleName, rTextName, rCellPos );
    maPreviousNote = aNewEntry;
    maNoteStyles.push_back( aNewEntry );
}

void ScSheetSaveData::AddNoteContentStyle( sal_uInt16 nFamily, const rtl::OUString& rName, const ScAddress& rCellPos, const ESelection& rSelection )
{
    if ( nFamily == XML_STYLE_FAMILY_TEXT_PARAGRAPH )
        maNoteParaStyles.push_back( ScTextStyleEntry( rName, rCellPos, rSelection ) );
    else
        maNoteTextStyles.push_back( ScTextStyleEntry( rName, rCellPos, rSelection ) );
}

void ScSheetSaveData::AddTextStyle( const rtl::OUString& rName, const ScAddress& rCellPos, const ESelection& rSelection )
{
    maTextStyles.push_back( ScTextStyleEntry( rName, rCellPos, rSelection ) );
}

void ScSheetSaveData::BlockSheet( sal_Int32 nTab )
{
    if ( nTab >= (sal_Int32)maBlocked.size() )
        maBlocked.resize( nTab + 1, false );        // fill vector with "false" entries

    maBlocked[nTab] = true;
}

bool ScSheetSaveData::IsSheetBlocked( sal_Int32 nTab ) const
{
    if ( nTab < (sal_Int32)maBlocked.size() )
        return maBlocked[nTab];
    else
        return false;
}

void ScSheetSaveData::AddStreamPos( sal_Int32 nTab, sal_Int32 nStartOffset, sal_Int32 nEndOffset )
{
    if ( nTab >= (sal_Int32)maStreamEntries.size() )
        maStreamEntries.resize( nTab + 1 );

    maStreamEntries[nTab] = ScStreamEntry( nStartOffset, nEndOffset );
}

void ScSheetSaveData::StartStreamPos( sal_Int32 nTab, sal_Int32 nStartOffset )
{
    DBG_ASSERT( mnStartTab < 0, "StartStreamPos without EndStreamPos" );

    mnStartTab = nTab;
    mnStartOffset = nStartOffset;
}

void ScSheetSaveData::EndStreamPos( sal_Int32 nEndOffset )
{
    if ( mnStartTab >= 0 )
    {
        AddStreamPos( mnStartTab, mnStartOffset, nEndOffset );
        mnStartTab = -1;
        mnStartOffset = -1;
    }
}

void ScSheetSaveData::GetStreamPos( sal_Int32 nTab, sal_Int32& rStartOffset, sal_Int32& rEndOffset ) const
{
    if ( nTab < (sal_Int32)maStreamEntries.size() )
    {
        const ScStreamEntry& rEntry = maStreamEntries[nTab];
        rStartOffset = rEntry.mnStartOffset;
        rEndOffset = rEntry.mnEndOffset;
    }
    else
        rStartOffset = rEndOffset = -1;
}

bool ScSheetSaveData::HasStreamPos( sal_Int32 nTab ) const
{
    sal_Int32 nStartOffset = -1;
    sal_Int32 nEndOffset = -1;
    GetStreamPos( nTab, nStartOffset, nEndOffset );
    return ( nStartOffset >= 0 && nEndOffset >= 0 );
}

void ScSheetSaveData::ResetSaveEntries()
{
    maSaveEntries.clear();
}

void ScSheetSaveData::AddSavePos( sal_Int32 nTab, sal_Int32 nStartOffset, sal_Int32 nEndOffset )
{
    if ( nTab >= (sal_Int32)maSaveEntries.size() )
        maSaveEntries.resize( nTab + 1 );

    maSaveEntries[nTab] = ScStreamEntry( nStartOffset, nEndOffset );
}

void ScSheetSaveData::UseSaveEntries()
{
    maStreamEntries = maSaveEntries;
}

void ScSheetSaveData::StoreInitialNamespaces( const SvXMLNamespaceMap& rNamespaces )
{
    // the initial namespaces are just removed from the list of loaded namespaces,
    // so only a hash_set of the prefixes is needed.

    const NameSpaceHash& rNameHash = rNamespaces.GetAllEntries();
    NameSpaceHash::const_iterator aIter = rNameHash.begin(), aEnd = rNameHash.end();
    while (aIter != aEnd)
    {
        maInitialPrefixes.insert( aIter->first );
        ++aIter;
    }
}

void ScSheetSaveData::StoreLoadedNamespaces( const SvXMLNamespaceMap& rNamespaces )
{
    // store the loaded namespaces, so the prefixes in copied stream fragments remain valid

    const NameSpaceHash& rNameHash = rNamespaces.GetAllEntries();
    NameSpaceHash::const_iterator aIter = rNameHash.begin(), aEnd = rNameHash.end();
    while (aIter != aEnd)
    {
        // ignore the initial namespaces
        if ( maInitialPrefixes.find( aIter->first ) == maInitialPrefixes.end() )
        {
            const NameSpaceEntry& rEntry = aIter->second.getBody();
            maLoadedNamespaces.push_back( ScLoadedNamespaceEntry( rEntry.sPrefix, rEntry.sName, rEntry.nKey ) );
        }
        ++aIter;
    }
}

bool lcl_NameInHash( const NameSpaceHash& rNameHash, const rtl::OUString& rName )
{
    NameSpaceHash::const_iterator aIter = rNameHash.begin(), aEnd = rNameHash.end();
    while (aIter != aEnd)
    {
        if ( aIter->second->sName == rName )
            return true;

        ++aIter;
    }
    return false;   // not found
}

bool ScSheetSaveData::AddLoadedNamespaces( SvXMLNamespaceMap& rNamespaces ) const
{
    // Add the loaded namespaces to the name space map.

    // first loop: only look for conflicts
    // (if the loaded namespaces were added first, this might not be necessary)
    const NameSpaceHash& rNameHash = rNamespaces.GetAllEntries();
    std::vector<ScLoadedNamespaceEntry>::const_iterator aIter = maLoadedNamespaces.begin();
    std::vector<ScLoadedNamespaceEntry>::const_iterator aEnd = maLoadedNamespaces.end();
    while (aIter != aEnd)
    {
        NameSpaceHash::const_iterator aHashIter = rNameHash.find( aIter->maPrefix );
        if ( aHashIter == rNameHash.end() )
        {
            if ( lcl_NameInHash( rNameHash, aIter->maName ) )
            {
                // a second prefix for the same name would confuse SvXMLNamespaceMap lookup,
                // so this is also considered a conflict
                return false;
            }
        }
        else if ( aHashIter->second->sName != aIter->maName )
        {
            // same prefix, but different name: loaded namespaces can't be used
            return false;
        }
        ++aIter;
    }

    // only if there were no conflicts, add the entries that aren't in the map already
    // (the key is needed if the same namespace is added later within an element)
    aIter = maLoadedNamespaces.begin();
    while (aIter != aEnd)
    {
        NameSpaceHash::const_iterator aHashIter = rNameHash.find( aIter->maPrefix );
        if ( aHashIter == rNameHash.end() )
            rNamespaces.Add( aIter->maPrefix, aIter->maName, aIter->mnKey );
        ++aIter;
    }

    return true;    // success
}

bool ScSheetSaveData::IsInSupportedSave() const
{
    return mbInSupportedSave;
}

void ScSheetSaveData::SetInSupportedSave( bool bSet )
{
    mbInSupportedSave = bSet;
}

