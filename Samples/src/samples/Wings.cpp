/*
 * Copyright (c) 2014 David Wicks, sansumbrella.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Wings.h"

using namespace choreograph;
using namespace cinder;

void Wings::setup()
{

  timeline().setDefaultRemoveOnFinish( false );

  timeline().apply( &position ).set( vec2( 0, app::getWindowHeight() / 2 ) )
    .then<RampTo>( vec2( app::getWindowWidth(), app::getWindowHeight() ), 1.0f, EaseInOutQuad() );
}

void Wings::connect( ci::app::WindowRef window )
{
  storeConnection( window->getSignalMouseDown().connect( [this] ( const app::MouseEvent &event ) {
    mMouseDown = true;
    float t = (event.getPos().x / (float) app::getWindowWidth());
    timeline().jumpTo( t );
  } ) );
  storeConnection( window->getSignalMouseUp().connect( [this] ( const app::MouseEvent &event ) { mMouseDown = false; } ) );

  storeConnection( window->getSignalMouseDrag().connect( [this] ( const app::MouseEvent &event ) {
    float t = (event.getPos().x / (float) app::getWindowWidth());
    timeline().jumpTo( t );
  }) );

  storeConnection( window->getSignalKeyDown().connect( [this] ( const app::KeyEvent &event ) {
    position.disconnect();
  }  ) );
}

void Wings::update( Time dt )
{
  if( ! mMouseDown )
  {
    timeline().step( dt );
  }
}

void Wings::draw()
{
  gl::ScopedColor color( Color::white() );
  gl::drawSolidCircle( position, 100.0f );
}
