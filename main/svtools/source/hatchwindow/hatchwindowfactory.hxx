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



#ifndef _XHATCHWINDOWFACTORY_HXX_
#define _XHATCHWINDOWFACTORY_HXX_

#include <com/sun/star/embed/XHatchWindowFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>


#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase2.hxx>
#endif


class OHatchWindowFactory : public ::cppu::WeakImplHelper2<
												::com::sun::star::embed::XHatchWindowFactory,
												::com::sun::star::lang::XServiceInfo >
{
	::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

public:
	OHatchWindowFactory(
		const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
	: m_xFactory( xFactory )
	{
		OSL_ENSURE( xFactory.is(), "No service manager is provided!\n" );
	}

	static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL impl_staticGetSupportedServiceNames();

	static ::rtl::OUString SAL_CALL impl_staticGetImplementationName();

	static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
		impl_staticCreateSelfInstance( 
			const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );


	// XHatchWindowFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XHatchWindow > SAL_CALL createHatchWindowInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent, const ::com::sun::star::awt::Rectangle& aBounds, const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::uno::RuntimeException);

	// XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

};

#endif

