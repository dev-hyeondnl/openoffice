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


#ifndef _CONNECTIVITY_ADO_AOLEWRAP_HXX_
#define _CONNECTIVITY_ADO_AOLEWRAP_HXX_

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <map>
#include <vector>
#include "connectivity/StdTypeDefs.hxx"

namespace rtl
{
	class OUString;
}
namespace connectivity
{
	namespace ado
	{
		class OLEVariant;
		class WpBase
		{
		protected:
			IDispatch* pIUnknown;

			void setIDispatch(IDispatch* _pIUnknown);
		public:
			WpBase();
			WpBase(IDispatch* pInt);
			//inline
			WpBase& operator=(const WpBase& rhs);
			WpBase& operator=(IDispatch* rhs);
			WpBase(const WpBase& aWrapper);
			virtual ~WpBase();
			void clear();


			sal_Bool IsValid() const;
			operator IDispatch*();

		};
		//////////////////////////////////////////////////////////////////////////
		//
		// Template-Klasse WpOLEBase<class T>
		// ==================================
		//
        // Objekte dieser Klasse haelt einen Zeiger auf ein Interface vom Typ T.
		// Es gibt Konstruktoren und Zuweisungsoperator die sicherstellen, dass
		// AddRef() und Release() entsprechend den COM-Konventionen gerufen werden.
		// Ein Objekt kann auch keinen Zeiger halten (Nullzeiger), dann ergibt
		// der Aufruf von IsValid() FALSE.
		//
		// Um effizientes pass-by-value machen zu koennen, ist diese (ebenso wie die
		// abgeleiteten Klassen) eine ganz schmale Wrapper-Klasse unter Vermeidung
		// virtueller Methoden und mit Inlining.

		//------------------------------------------------------------------------
		template<class T> class WpOLEBase : public WpBase
		{
		protected:
			T* pInterface;

		public:
			WpOLEBase(T* pInt = NULL) : WpBase(pInt),pInterface(pInt){}


			//inline
			WpOLEBase<T>& operator=(const WpOLEBase<T>& rhs)
			{
				WpBase::operator=(rhs);
				pInterface = rhs.pInterface;
				return *this;
			};

			WpOLEBase<T>& operator=(T* rhs)
			{
				WpBase::operator=(rhs);
				pInterface = rhs.pInterface;
				return *this;
			}

			WpOLEBase(const WpOLEBase<T>& aWrapper)
			{
				operator=(aWrapper);
			}

			virtual ~WpOLEBase()
			{
			}

			operator T*() const { return static_cast<T*>(pInterface); }
			void setWithOutAddRef(T* _pInterface)
			{
				pInterface = _pInterface;
				WpBase::setIDispatch(_pInterface);
			}
		};


		//////////////////////////////////////////////////////////////////////////
		//
		// Template-Klasse WpOLECollection<class Ts, class T, class WrapT>
		// ===============================================================
		//
		// Diese Klasse, welche sich von WpOLEBase<Ts> ableitet, abstrahiert die
		// den DAO-Collections gemeinsamen Eigenschaften:
		//
        // Sie werden ueber ein Interface Ts (etwa: DAOFields) angesprochen
		// und koennen ueber get_Item (hier:GetItem) Items des Typs T (genauer:
		// mit Interface T, etwa DAOField) herausgeben.
		//
		// Diese Wrapperklasse gibt aber nicht ein Interface T heraus,
        // sondern ein Objekt der Klasse WrapT. Dieses muss eine Konstruktion
		// durch T zulassen, vorzugsweise ist es von WpOLEBase<T> abgeleitet.
		//

		//------------------------------------------------------------------------
		template<class Ts, class T, class WrapT> class WpOLECollection : public WpOLEBase<Ts>
		{
		public:
			using WpOLEBase<Ts>::pInterface;
			using WpOLEBase<Ts>::IsValid;
			// Konstruktoren, operator=
			// diese rufen nur die Oberklasse
			WpOLECollection(Ts* pInt=NULL):WpOLEBase<Ts>(pInt){}
			WpOLECollection(const WpOLECollection& rhs){operator=(rhs);}
			inline WpOLECollection& operator=(const WpOLECollection& rhs)
				{WpOLEBase<Ts>::operator=(rhs); return *this;};

			//////////////////////////////////////////////////////////////////////

			inline void Refresh(){pInterface->Refresh();}

			inline sal_Int32 GetItemCount()	const
			{
				sal_Int32 nCount = 0;
				return pInterface ? (SUCCEEDED(pInterface->get_Count(&nCount)) ? nCount : sal_Int32(0)) : sal_Int32(0);
			}

			inline WrapT GetItem(sal_Int32 index) const
			{
				OSL_ENSURE(index >= 0 && index<GetItemCount(),"Wrong index for field!");
				T* pT = NULL;
				WrapT aRet(NULL);
				if(SUCCEEDED(pInterface->get_Item(OLEVariant(index), &pT)))
					aRet.setWithOutAddRef(pT);
				return aRet;
			}

			inline WrapT GetItem(const OLEVariant& index) const
			{
				T* pT = NULL;
				WrapT aRet(NULL);
				if(SUCCEEDED(pInterface->get_Item(index, &pT)))
					aRet.setWithOutAddRef(pT);
				return aRet;
			}

			inline WrapT GetItem(const ::rtl::OUString& sStr) const
			{
				WrapT aRet(NULL);
				T* pT = NULL;
				if (FAILED(pInterface->get_Item(OLEVariant(sStr), &pT)))
				{
#if OSL_DEBUG_LEVEL > 0
					::rtl::OString sTemp("Unknown Item: ");
					sTemp += ::rtl::OString(sStr.getStr(),sStr.getLength(),osl_getThreadTextEncoding());
					OSL_ENSURE(0,sTemp);
#endif
				}
				else
					aRet.setWithOutAddRef(pT);
				return aRet;
			}
			inline void fillElementNames(TStringVector& _rVector)
			{
				if(IsValid())
				{
					Refresh();
					sal_Int32 nCount = GetItemCount();
					_rVector.reserve(nCount);
					for(sal_Int32 i=0;i< nCount;++i)
					{
						WrapT aElement = GetItem(i);
						if(aElement.IsValid())
							_rVector.push_back(aElement.get_Name());
					}
				}
			}
		};

		template<class Ts, class T, class WrapT> class WpOLEAppendCollection:
				public WpOLECollection<Ts,T,WrapT>
		{

		public:
			// Konstruktoren, operator=
			// diese rufen nur die Oberklasse
			using WpOLEBase<Ts>::pInterface;
			WpOLEAppendCollection(Ts* pInt=NULL):WpOLECollection<Ts,T,WrapT>(pInt){}
			WpOLEAppendCollection(const WpOLEAppendCollection& rhs){ operator=(rhs); }
			inline WpOLEAppendCollection& operator=(const WpOLEAppendCollection& rhs)
				{WpOLEBase<Ts>::operator=(rhs); return *this;};
			//////////////////////////////////////////////////////////////////////

			inline sal_Bool Append(const WrapT& aWrapT)
			{
				return SUCCEEDED(pInterface->Append(OLEVariant((T*)aWrapT)));
			};

			inline sal_Bool Delete(const ::rtl::OUString& sName)
			{
				return SUCCEEDED(pInterface->Delete(OLEVariant(sName)));
			};


		};
	}
}
#endif // _CONNECTIVITY_ADO_AOLEWRAP_HXX_

