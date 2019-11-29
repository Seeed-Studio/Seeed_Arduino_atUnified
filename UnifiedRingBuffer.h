/*
  Copyright (c) 2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#pragma once
#include <stdint.h>
#include <setjmp.h>
// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which head is the index of the location
// to which to write the next incoming character and tail is the index of the
// location from which to read.

template<class type>
struct UnifiedRingBuffer{
private:
    int          N;
    type      *  _aucBuffer;
    volatile int _iHead;
    volatile int _iTail;
public:
    UnifiedRingBuffer(){
        _aucBuffer = nullptr;
        N = 0;
    }
    UnifiedRingBuffer(int size);
    ~UnifiedRingBuffer(){
        if (_aucBuffer){
            delete [] _aucBuffer;
            _aucBuffer = nullptr;
            N = 0;
        }
    }
    operator bool(){
        return _aucBuffer != nullptr;
    }
    void setup(int size);
    void store_char(uint8_t c);
    void clear();
    int read_char();
    int available();
    int availableForStore();
    int peek();
    bool isFull();

  private:
    int nextIndex(int index);
};


template <class type>
void UnifiedRingBuffer<type>::setup(int size){
    ~UnifiedRingBuffer();
    N = size;
    _aucBuffer = new type[size];
    if (_aucBuffer){
        clear();
    }
    else{
        N = 0;
    }
}

template <class type>
UnifiedRingBuffer<type>::UnifiedRingBuffer(int size) : 
    _aucBuffer(nullptr){ // to avoid destructor to release a error pointer.
    setup(size);
}

template <class type>
void UnifiedRingBuffer<type>::store_char( uint8_t c )
{
  int i = nextIndex(_iHead);

  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if ( i != _iTail )
  {
    _aucBuffer[_iHead] = c ;
    _iHead = i ;
  }
}

template <class type>
void UnifiedRingBuffer<type>::clear()
{
  _iHead = 0;
  _iTail = 0;
}

template <class type>
int UnifiedRingBuffer<type>::read_char()
{
  if(_iTail == _iHead)
    return -1;

  uint8_t value = _aucBuffer[_iTail];
  _iTail = nextIndex(_iTail);

  return value;
}

template <class type>
int UnifiedRingBuffer<type>::available()
{
  int delta = _iHead - _iTail;

  if(delta < 0)
    return N + delta;
  else
    return delta;
}

template <class type>
int UnifiedRingBuffer<type>::availableForStore()
{
  if (_iHead >= _iTail)
    return N - 1 - _iHead + _iTail;
  else
    return _iTail - _iHead - 1;
}

template <class type>
int UnifiedRingBuffer<type>::peek()
{
  if(_iTail == _iHead)
    return -1;

  return _aucBuffer[_iTail];
}

template <class type>
int UnifiedRingBuffer<type>::nextIndex(int index)
{
  return (uint32_t)(index + 1) % N;
}

template <class type>
bool UnifiedRingBuffer<type>::isFull()
{
  return (nextIndex(_iHead) == _iTail);
}
