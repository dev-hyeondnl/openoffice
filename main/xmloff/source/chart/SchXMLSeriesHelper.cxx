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
#include "precompiled_xmloff.hxx"

#include "SchXMLSeriesHelper.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

// header for define RTL_CONSTASCII_USTRINGPARAM
#include <rtl/ustring.h>
// header for define DBG_ERROR1
#include <tools/debug.hxx>

#include <typeinfo>

using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringToOString;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

// ----------------------------------------

::std::vector< Reference< chart2::XDataSeries > >
    SchXMLSeriesHelper::getDataSeriesFromDiagram(
        const Reference< chart2::XDiagram > & xDiagram )
{
    ::std::vector< Reference< chart2::XDataSeries > > aResult;

    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
        {
            Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[i], uno::UNO_QUERY_THROW );
            Sequence< Reference< chart2::XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
            for( sal_Int32 j=0; j<aChartTypeSeq.getLength(); ++j )
            {
                Reference< chart2::XDataSeriesContainer > xDSCnt( aChartTypeSeq[j], uno::UNO_QUERY_THROW );
                Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries() );
                ::std::copy( aSeriesSeq.getConstArray(), aSeriesSeq.getConstArray() + aSeriesSeq.getLength(),
                             ::std::back_inserter( aResult ));
            }
        }
    }
    catch( uno::Exception & ex )
    {
        (void)ex; // avoid warning for pro build

        OSL_ENSURE( false, OUStringToOString( OUString(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "Exception caught. Type: " )) +
                        OUString::createFromAscii( typeid( ex ).name()) +
                        OUString( RTL_CONSTASCII_USTRINGPARAM( ", Message: " )) +
                        ex.Message), RTL_TEXTENCODING_ASCII_US ).getStr());

    }

    return aResult;
}

::std::map< Reference< chart2::XDataSeries >, sal_Int32 > SchXMLSeriesHelper::getDataSeriesIndexMapFromDiagram(
        const Reference< chart2::XDiagram > & xDiagram )
{
    ::std::map< Reference< chart2::XDataSeries >, sal_Int32 > aRet;

    sal_Int32 nIndex=0;

    ::std::vector< Reference< chart2::XDataSeries > > aSeriesVector( SchXMLSeriesHelper::getDataSeriesFromDiagram( xDiagram ));
    for( ::std::vector< Reference< chart2::XDataSeries > >::const_iterator aSeriesIt( aSeriesVector.begin() )
        ; aSeriesIt != aSeriesVector.end()
        ; aSeriesIt++, nIndex++ )
    {
        Reference< chart2::XDataSeries > xSeries( *aSeriesIt );
        if( xSeries.is() )
        {
            if( aRet.end() == aRet.find(xSeries) )
                aRet[xSeries]=nIndex;
        }
    }
    return aRet;
}

uno::Reference< chart2::XChartType > lcl_getChartTypeOfSeries(
								const uno::Reference< chart2::XDiagram >&   xDiagram
						      , const Reference< chart2::XDataSeries >& xSeries )
{
    if(!xDiagram.is())
        return 0;

	//iterate through the model to find the given xSeries
	//the found parent indicates the charttype

    //iterate through all coordinate systems
    uno::Reference< chart2::XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( !xCooSysContainer.is())
        return 0;

    uno::Sequence< uno::Reference< chart2::XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
    {
        uno::Reference< chart2::XCoordinateSystem > xCooSys( aCooSysList[nCS] );

        //iterate through all chart types in the current coordinate system
        uno::Reference< chart2::XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
        OSL_ASSERT( xChartTypeContainer.is());
        if( !xChartTypeContainer.is() )
            continue;
        uno::Sequence< uno::Reference< chart2::XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
        for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
        {
            uno::Reference< chart2::XChartType > xChartType( aChartTypeList[nT] );

            //iterate through all series in this chart type
            uno::Reference< chart2::XDataSeriesContainer > xDataSeriesContainer( xChartType, uno::UNO_QUERY );
            OSL_ASSERT( xDataSeriesContainer.is());
            if( !xDataSeriesContainer.is() )
                continue;

            uno::Sequence< uno::Reference< chart2::XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );
            for( sal_Int32 nS = 0; nS < aSeriesList.getLength(); ++nS )
            {
                Reference< chart2::XDataSeries > xCurrentSeries( aSeriesList[nS] );

                if( xSeries == xCurrentSeries )
                    return xChartType;
            }
        }
    }
	return 0;
}

bool SchXMLSeriesHelper::isCandleStickSeries(
                  const Reference< chart2::XDataSeries >& xSeries
                , const Reference< frame::XModel >& xChartModel )
{
    bool bRet = false;

    uno::Reference< chart2::XChartDocument > xNewDoc( xChartModel, uno::UNO_QUERY );
    if( xNewDoc.is() )
    {
        uno::Reference< chart2::XDiagram > xNewDiagram( xNewDoc->getFirstDiagram() );
        if( xNewDiagram.is() )
        {
            uno::Reference< chart2::XChartType > xChartType( lcl_getChartTypeOfSeries(
								        xNewDiagram, xSeries ) );
            if( xChartType.is() )
            {
                rtl::OUString aServiceName( xChartType->getChartType() );
                if( aServiceName.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.CandleStickChartType" ) ) ) )
                    bRet = true;
            }
        }
    }
    return bRet;
}

// static
Reference< chart2::XDataSeries > SchXMLSeriesHelper::getFirstCandleStickSeries(
    const Reference< chart2::XDiagram > & xDiagram  )
{
    Reference< chart2::XDataSeries > xResult;

    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 nCooSysIdx=0; !xResult.is() && nCooSysIdx<aCooSysSeq.getLength(); ++nCooSysIdx )
        {
            Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[nCooSysIdx], uno::UNO_QUERY_THROW );
            Sequence< Reference< chart2::XChartType > > aCTSeq( xCTCnt->getChartTypes());
            for( sal_Int32 nCTIdx=0; !xResult.is() && nCTIdx<aCTSeq.getLength(); ++nCTIdx )
            {
                if( aCTSeq[nCTIdx]->getChartType().equals(
                        ::rtl::OUString::createFromAscii("com.sun.star.chart2.CandleStickChartType")))
                {
                    Reference< chart2::XDataSeriesContainer > xSeriesCnt( aCTSeq[nCTIdx], uno::UNO_QUERY_THROW );
                    Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xSeriesCnt->getDataSeries() );
                    if( aSeriesSeq.getLength())
                        xResult.set( aSeriesSeq[0] );
                    break;
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        OSL_ENSURE( false, "Exception caught" );
    }
    return xResult;
}

//static
uno::Reference< beans::XPropertySet > SchXMLSeriesHelper::createOldAPISeriesPropertySet(
            const uno::Reference< chart2::XDataSeries >& xSeries
            , const uno::Reference< frame::XModel >& xChartModel )
{
    uno::Reference< beans::XPropertySet > xRet;

    if( xSeries.is() )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFactory( xChartModel, uno::UNO_QUERY );
            if( xFactory.is() )
            {
                xRet = uno::Reference< beans::XPropertySet >( xFactory->createInstance(
                    OUString::createFromAscii( "com.sun.star.comp.chart2.DataSeriesWrapper" ) ), uno::UNO_QUERY );
                Reference< lang::XInitialization > xInit( xRet, uno::UNO_QUERY );
                if(xInit.is())
                {
                    Sequence< uno::Any > aArguments(1);
                    aArguments[0]=uno::makeAny(xSeries);
                    xInit->initialize(aArguments);
                }
            }
        }
        catch( uno::Exception & rEx )
	    {
            (void)rEx; // avoid warning for pro build
            DBG_ERROR1( "Exception caught SchXMLSeriesHelper::createOldAPISeriesPropertySet: %s",
                        OUStringToOString( rEx.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        }
    }

    return xRet;
}

//static
uno::Reference< beans::XPropertySet > SchXMLSeriesHelper::createOldAPIDataPointPropertySet(
            const uno::Reference< chart2::XDataSeries >& xSeries
            , sal_Int32 nPointIndex
            , const uno::Reference< frame::XModel >& xChartModel )
{
    uno::Reference< beans::XPropertySet > xRet;

    if( xSeries.is() )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFactory( xChartModel, uno::UNO_QUERY );
            if( xFactory.is() )
            {
                xRet = uno::Reference< beans::XPropertySet >( xFactory->createInstance(
                    OUString::createFromAscii( "com.sun.star.comp.chart2.DataSeriesWrapper" ) ), uno::UNO_QUERY );
                Reference< lang::XInitialization > xInit( xRet, uno::UNO_QUERY );
                if(xInit.is())
                {
                    Sequence< uno::Any > aArguments(2);
                    aArguments[0]=uno::makeAny(xSeries);
                    aArguments[1]=uno::makeAny(nPointIndex);
                    xInit->initialize(aArguments);
                }
            }
        }
        catch( uno::Exception & rEx )
	    {
            (void)rEx; // avoid warning for pro build

            DBG_ERROR1( "Exception caught SchXMLSeriesHelper::createOldAPIDataPointPropertySet: %s",
                        OUStringToOString( rEx.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        }
    }

    return xRet;
}

