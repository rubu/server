/*
* Copyright (c) 2011 Sveriges Television AB <info@casparcg.com>
*
* This file is part of CasparCG (www.casparcg.com).
*
* CasparCG is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CasparCG is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CasparCG. If not, see <http://www.gnu.org/licenses/>.
*
* Author: Robert Nagy, ronag89@gmail.com
*/

#pragma once

#include <functional>
#include <vector>
#include <string>

namespace caspar {

/**
 * A tweener can be used for creating any kind of (image position, image fade
 * in/out, audio volume etc) transition, by invoking it for each temporal
 * timepoint when a tweened value is needed.
 *
 * For video the temporal resolution will usually be each frame or field (for
 * interlaced material).
 *
 * For audio the smoothest transitions will be generated by using the samplerate
 * as temporal resolution, but using the video frame/field rate is probably fine
 * most of the times and much less time consuming.
 */
class tweener
{
public:
	/**
	 * Constructor.
	 *
	 * @param name The name of the tween function to use.
	 */
	tweener(const std::wstring& name = L"linear");

	/**
	 * @return The possible tween function names. Some of them may also support
	 * 		   additional parameters appended to the name.
	 */
	static const std::vector<std::wstring>& names();

	/**
	 * Calculate a tweened value given a timepoint within the total duration
	 * and the starting value and the destination delta value.
	 *
	 * Usually b, c and d remains constant during a transition, while t changes
	 * for each temporal tweened value.
	 * 
	 * @param t	The timepoint within the total duration (0 <= n <= d).
	 * @param b	The starting value.
	 * @param c	The destination value delta from the starting value
	 * 	        (absolute destination value - b).
	 * @param d The total duration (when t = d, the destination value should
	 * 	        have been reached).
	 *
	 * @return The tweened value for the given timepoint. Can sometimes be less
	 * 	       than b or greater than b + c for some tweener functions.
	 */
	double operator()(double t, double b , double c, double d) const;

	bool operator==(const tweener& other) const;
	bool operator!=(const tweener& other) const;
private:
	std::function<double(double, double, double, double)>	func_;
	std::wstring											name_;
};

}
