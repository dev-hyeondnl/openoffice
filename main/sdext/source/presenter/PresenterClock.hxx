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



#ifndef SDEXT_PRESENTER_CLOCK_HXX
#define SDEXT_PRESENTER_CLOCK_HXX

#include "PresenterController.hxx"

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase4.hxx>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/thread.hxx>
#include <rtl/ref.hxx>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace {
    typedef cppu::WeakComponentImplHelper4<
        css::awt::XPaintListener,
        css::awt::XWindowListener,
        css::awt::XMouseListener,
        css::drawing::framework::XView
        > PresenterClockInterfaceBase;
}

namespace sdext { namespace presenter {


/** A clock that displays the current time.  This class is work in
    progress.  Future extensions may include
    other times like time since presentation started or remaining time.
    Painting of the clock is done by the inner Painer class which includes
    at the moment a simple analog and a simple digital clock.
*/
class PresenterClock
    : private ::cppu::BaseMutex,
      public PresenterClockInterfaceBase
{
public:
    static ::rtl::Reference<PresenterClock> Create (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::frame::XController>& rxController,
        const ::rtl::Reference<PresenterController>& rpPresenterController);

    virtual void SAL_CALL disposing (void);

    /** Callback for an external timer or thread that initiates updates when
        the time changes (seconds or minutes).
    */
    void UpdateTime (void);

    /** An internally used base class for different painters.
    */
    class Painter;


    // lang::XEventListener

    virtual void SAL_CALL 
        disposing (const css::lang::EventObject& rEventObject)
        throw (css::uno::RuntimeException);


    // XPaintListener

    virtual void SAL_CALL windowPaint (const css::awt::PaintEvent& rEvent)
        throw (css::uno::RuntimeException);


    // XWindowListener
    
    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);


    // XMouseListener

    virtual void SAL_CALL mousePressed (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseReleased (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);
    
    virtual void SAL_CALL mouseEntered (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);
    
    virtual void SAL_CALL mouseExited (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);
    

    // XResourceId

    virtual css::uno::Reference<css::drawing::framework::XResourceId> SAL_CALL getResourceId (void)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isAnchorOnly (void)
        throw (com::sun::star::uno::RuntimeException);

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewId;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    css::uno::Reference<css::drawing::framework::XPane> mxPane;
    ::rtl::Reference<PresenterController> mpPresenterController;
    bool mbIsResizePending;
    css::rendering::ViewState maViewState;
    css::rendering::RenderState maRenderState;
    /** A Timer is used for sampling the current time and schedule repaints
        when the minute or second (when these are displayed) values have changed.
    */
    class Timer;
    Timer* mpTimer;
    ::boost::scoped_ptr<Painter> mpClockPainter;
    /** 
        This is used for debugging to show one clock atop another to compare
        the output of the painters.
    */
    ::boost::scoped_ptr<Painter> mpClockPainter2;
    int mnMode;
    sal_Int32 mnHour;
    sal_Int32 mnMinute;
    sal_Int32 mnSecond;

    bool mbIsShowSeconds;

    /** Use the static Create() method for creating a new PresenterClock
        object.
    */
    PresenterClock (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::frame::XController>& rxController,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterClock (void);

    void LateInit (void);
    void Resize (void);
    void Paint (const css::awt::Rectangle& rUpdateRectangle);
    css::uno::Reference<css::rendering::XPolyPolygon2D> CreatePolygon (
        const css::awt::Rectangle& rBox);
    void Clear (const css::uno::Reference<css::rendering::XPolyPolygon2D>& rxUpdatePolygon);
    void SetMode (sal_Int32 nMode);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void)
        throw (css::lang::DisposedException);
};

} } // end of namespace ::sdext::presenter

#endif
