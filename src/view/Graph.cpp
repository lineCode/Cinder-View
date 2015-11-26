/*
 Copyright (c) 2014, Richard Eakin - All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided
 that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "view/Graph.h"
#include "view/View.h"

#include "cinder/app/AppBase.h"

using namespace ci;
using namespace std;

namespace view {

Graph::Graph( const ci::app::WindowRef &window )
	: mWindow( window )
{
	if( ! mWindow ) {
		auto app = app::AppBase::get();
		if( ! app ) {
			throw GraphExc( "Running app-less, must provide an app::Window" );
		}

		mWindow = app->getWindow();
		mMultiTouchEnabled = app->isMultiTouchEnabled();
	}

	mRenderer = make_shared<view::Renderer>();
}

Graph::~Graph()
{
	if( ! mEventConnections.empty() )
		disconnectEvents();
}

//ViewRef Graph::getRootView()
//{
//	if( ! mRootView ) {
//		mRootView.reset( new View( mWindow->getBounds() ) );
//	}
//}

void Graph::update()
{
	propagateUpdate();
}

void Graph::draw()
{
	propagateDraw();
}

void Graph::connectTouchEvents( int priority )
{
	mEventSlotPriority = priority;

	if( ! mEventConnections.empty() )
		disconnectEvents();

	if( mMultiTouchEnabled ) {
		mEventConnections.push_back( mWindow->getSignalTouchesBegan().connect( mEventSlotPriority,	bind( &View::propagateTouchesBegan, this, placeholders::_1 ) ) );
		mEventConnections.push_back( mWindow->getSignalTouchesMoved().connect( mEventSlotPriority,	bind( &View::propagateTouchesMoved, this, placeholders::_1 ) ) );
		mEventConnections.push_back( mWindow->getSignalTouchesEnded().connect( mEventSlotPriority,	bind( &View::propagateTouchesEnded, this, placeholders::_1 ) ) );
	}
	else {
		mEventConnections.push_back( mWindow->getSignalMouseDown().connect( mEventSlotPriority, [&]( app::MouseEvent &event ) {
			app::TouchEvent touchEvent( event.getWindow(), vector<app::TouchEvent::Touch>( 1, app::TouchEvent::Touch( event.getPos(), vec2( 0 ), 0, 0, &event ) ) );
			propagateTouchesBegan( touchEvent );
			event.setHandled( touchEvent.isHandled() );
		} ) );
		mEventConnections.push_back( mWindow->getSignalMouseDrag().connect( mEventSlotPriority, [&]( app::MouseEvent &event ) {
			app::TouchEvent touchEvent( event.getWindow(), vector<app::TouchEvent::Touch>( 1, app::TouchEvent::Touch( event.getPos(), vec2( 0 ), 0, 0, &event ) ) );
			propagateTouchesMoved( touchEvent );
			event.setHandled( touchEvent.isHandled() );
		} ) );
		mEventConnections.push_back( mWindow->getSignalMouseUp().connect( mEventSlotPriority, [&]( app::MouseEvent &event ) {
			app::TouchEvent touchEvent( event.getWindow(), vector<app::TouchEvent::Touch>( 1, app::TouchEvent::Touch( event.getPos(), vec2( 0 ), 0, 0, &event ) ) );
			propagateTouchesEnded( touchEvent );
			event.setHandled( touchEvent.isHandled() );
		} ) );
	}
}

void Graph::disconnectEvents()
{
	for( auto &connection : mEventConnections )
		connection.disconnect();

	mEventConnections.clear();
}

} // namespace view
