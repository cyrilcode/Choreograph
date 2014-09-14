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

#pragma once

#include "Phrase.hpp"
#include <cmath>

namespace choreograph
{

/**
 A Sequence of motions.
 Our essential compositional tool, describing all the transformations to one element.
 A kind of platonic idea of an animation sequence; this describes a motion without giving it an output.
*/
template<typename T, typename PhraseT=Phrase<T>>
class Sequence
{
public:
  // Sequences always need to have some valid value.
  Sequence() = delete;
  using SequenceT = Sequence<T, PhraseT>;

  //! Construct a Sequence with an initial \a value.
  explicit Sequence( const T &value ):
    _initial_value( value )
  {}

  //! Returns the Sequence value at \a atTime.
  T getValue( float atTime ) const;

  float getTimeWrapped( float time, float inflectionPoint = 0.0f ) const
  {
    if( time > getDuration() ) {
      return inflectionPoint + std::fmodf( time, getDuration() - inflectionPoint );
    }
    else {
      return time;
    }
  }

  //! Returns the Sequence value at \a time, looping past the end from inflection point to the end.
  T getValueWrapped( float time, float inflectionPoint = 0.0f ) const
  {
    return getValue( getTimeWrapped( time, inflectionPoint ) );
  }

  //! Set current value. An instantaneous hold.
  SequenceT& set( const T &value )
  {
    if( _segments.empty() ) {
      _initial_value = value;
    }
    else {
      hold( value, 0.0f );
    }
    return *this;
  }

  //! Returns a copy of this sequence. Useful if you want to make a base animation and modify that.
  std::shared_ptr<Sequence<T>> copy() const { return std::make_shared<Sequence<T>>( *this ); }

  //! Hold on current end value for \a duration seconds.
  SequenceT& wait( float duration ) { return hold( duration ); }

  //! Hold on current end value for \a duration seconds.
  SequenceT& hold( float duration )
  {
    return hold( endValue(), duration );
  }

  //! Hold on \a value for \a duration seconds.
  SequenceT& hold( const T &value, float duration )
  {
    Position<T> start{ value, _duration };
    Position<T> end{ value, _duration + duration };
    Phrase<T> phrase( start, end, &easeHold );

    _segments.push_back( phrase );
    _duration = phrase.getEndTime();

    return *this;
  }

  //! Animate to \a value over \a duration seconds using \a ease easing.
  SequenceT& rampTo( const T &value, float duration, const EaseFn &ease = &easeNone )
  {
    Position<T> start{ endValue(), _duration };
    Position<T> end{ value, _duration + duration };
    Phrase<T> phrase( start, end, ease );

    _segments.push_back( phrase );

    _duration = phrase.getEndTime();

    return *this;
  }

  template<typename... Args>
  SequenceT& then( const T &value, float duration, Args... args )
  {
    Position<T> start{ endValue(), _duration };
    Position<T> end{ value, _duration + duration };
    PhraseT phrase( start, end, args... );

    _segments.push_back( phrase );
    _duration = phrase.getEndTime();

    return *this;
  }

  Sequence<T, PhraseT>& then( PhraseT phrase )
  {
    phrase.setStartValue( endValue() );
    phrase.shiftStartTimeTo( _duration );
    _duration = phrase.getEndTime();

    _segments.push_back( phrase );

    return *this;
  }

  //! Sets the ease function of the last Phrase in the Sequence.
  SequenceT& ease( const EaseFn &easeFn ) {
    if( ! _segments.empty() ) {
      _segments.back().motion = easeFn;
    }
    return *this;
  }

  //! Returns the number of seconds required to move through all Phrases.
  float getDuration() const { return _duration; }

  //! Returns the value at the end of the Sequence.
  T endValue() const { return _segments.empty() ? _initial_value : _segments.back().getEndValue(); }

  //! Returns the value at the beginning of the Sequence.
  T initialValue() const { return _initial_value; }

private:
  std::vector<PhraseT>  _segments;
  T                       _initial_value;
  float                   _duration = 0.0f;

  friend class Timeline;
};

//! Returns the value of this sequence for a given point in time.
// Would be nice to have a constant-time check (without a while loop).
template<typename T, typename PhraseT>
T Sequence<T, PhraseT>::getValue( float atTime ) const
{
  if( atTime < 0.0f )
  {
    return _initial_value;
  }
  else if ( atTime >= _duration )
  {
    return endValue();
  }

  auto iter = _segments.begin();
  while( iter < _segments.end() ) {
    if( (*iter).getEndTime() > atTime )
    {
      return (*iter).getValue( atTime );
    }
    ++iter;
  }
  // past the end, get the final value
  // this should be unreachable, given that we return early if time >= duration
  return endValue();
}

template<typename T, typename PhraseT = Phrase<T>>
using SequenceRef = std::shared_ptr<Sequence<T, PhraseT>>;

} // namespace choreograph
