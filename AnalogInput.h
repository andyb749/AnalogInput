//***************************************************************************
//
//  File Name :		analog.h
//
//  Project :		Analog input library for the Atmel 8 bit AVR MCU
//
//  Purpose :		Easy access to analog input pins on Atmel 8 bit AVR MCUs
//
// The MIT License (MIT)
//
// Copyright (c) 2013-2015 Andy Burgess
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//  Revisions :
//
//      see rcs below
//
//***************************************************************************


#ifndef ANALOG_H_
#define ANALOG_H_

#include <avr/io.h>
#include <stdint.h>


//! \brief Template class for an analog input
//! \details This class encapsulates an analog input.  There are
//! two different ways to construct the object depending on its intended use:
//! + read the raw input
//! + read the value in engineering units
//! + read the value as a sector number
//! \par Raw input
//! Use the AnalogInput<chan>() version of the constructor.  Read the input using
//! the read() method or the = operator overload.  This constructor will set default
//! values of 16 for number of sectors and set an engineering range of 0.0 to 100.0 (%).
//! \par Engineering units
//! Use the AnalogInput<chan>(min, max) version of the constructor.  Read the
//! input using readEng method which will return the input scaled between the min and max.
//! \par Sector number
//! Use the AnalogInput<chan>(sectors) version of the constructor.  Read the input
//! using the readSector() method which will return a number between 0 and sectors - 1.
//! Imagine that the analog input range is split into sectors, this method returns the
//! sector that the analog value is in.
//! \par
//! This template class sets the analog reference as AVCC and the prescaler to divide
//! by 128.  Later versions of the library may allow these to be determined by the end
//! user.
//! \param chan The channel number 0-15
template <uint8_t chan>
class AnalogInput
{
	private: class _init
	{
		public:
		_init()
		{
			ADCSRA |= (_BV(ADEN) | 0x07);			// enable module and pre-scaler = / 128
			ADMUX |= _BV(REFS0);					// set up the analog reference as AVCC
		}

	} _initializer;

public:
private:
	const float mult;
	const float offs;
	const uint16_t div;

public:
	//! \brief Initialises a new instance of the AnalogInput class using
	//! default values for engineering ranges and sectors.
	//! \details This overload uses 0.0 - 100.0 for the engineering range
	//! and 16 for the number of sectors.
	inline AnalogInput () : mult(100.0/1023.0), offs(0.0), div(1023/15)
	{
		setup();
	}


	//! \brief Initialises a new instance of the AnalogInput class using
	//! the user supplied number of sectors.
	//! \details This overload allows a user to split the analog input
	//! range into sectors.  The readSector() method will return the
	//! sector number of the analog input.
	//! \param sectors Number of sectors for the analogue input
	inline AnalogInput (uint8_t sectors) __attribute__((always_inline)) : mult(100.0 / 1023.0), offs(0.0), div(1023 / (sectors-1))
	{
		setup();
	}


	//! \brief Initialises a new instance of the AnalogInput class using
	//! the user supplied engineering range.
	//! \details This overload allows a user to specify an engineering
	//! range for the analogue input.  The readEng() method will return
	//! the analog input ranged between the zero and span values.
	//! The engineering value is calculated from y = mx + c which is the
	//! equation of a straight line.  m and c are calculated from zero
	//! and span by the pre-compiler and cached for later.
	//! \param zero The engineering value that corresponds to an analog
	//! input of 0V.
	//! \param span The engineering value that corresponds to an analog
	//! input of 5V.
	inline AnalogInput (float zero, float span) __attribute__((always_inline)) : mult((span-zero) / 1023.0), offs(zero), div(1023/15)
	{
		setup();
//		this->mult = (span - zero) / 1023.0;
//		this->offs = zero;
	}


	//! \brief Reads the analog input as an uint16_t
	//! \details Reads the analog input pin and returns it
	//! as an uint16_t between 0 and 1023
	//! \returns The raw value of the analog input
	inline uint16_t read() __attribute__ ((always_inline))
	{
		ADMUX &= ~(0x07);						// clean low 3 bits
		ADMUX |= (chan & 0x07);					// low 3 bits of chan are used to set MUX3:0

		#if defined(MUX5)
		ADCSRB &= ~_BV(MUX5);					// clear MUX5
		ADCSRB |= ((chan > 7) << MUX5);			// set MUX5 for chans 8-15
		#endif

		ADCSRA |= _BV(ADSC);					// start the conversion
		loop_until_bit_is_clear (ADCSRA, ADSC);	// wait for the conversion to complete

		return ADC;								// read the result and return
	}


	//! \brief Operator overload to read the analog input.
	inline operator uint16_t () __attribute__ ((always_inline))
	{
		return read();
	}


	//! \brief Returns the analog input in engineering units.
	//! \returns The value of the input scaled into engineering units.
	inline float readEng()
	{
		return (float) read() * this->mult + this->offs;
	}


	//! \brief Returns the analog input expressed as a sector.
	//! \returns The sector number of the analog input.
	inline uint8_t readSector()
	{
		return read() / this->div;
	}

private:

	inline void setup ()
	{
		// digital input disable
		#if (chan < 8)
		DIDR0 |= _BV(chan&0x07);
		#else
		DIDR2 |= _BV(chan&0x07);
		#endif
	}

};


#endif /* ANALOG_H_ */