//=================================================================================================
/*!
//  \file blaze/math/typetraits/IsRowVector.h
//  \brief Header file for the IsRowVector type trait
//
//  Copyright (C) 2012-2018 Klaus Iglberger - All Rights Reserved
//
//  This file is part of the Blaze library. You can redistribute it and/or modify it under
//  the terms of the New (Revised) BSD License. Redistribution and use in source and binary
//  forms, with or without modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//     of conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//  3. Neither the names of the Blaze development group nor the names of its contributors
//     may be used to endorse or promote products derived from this software without specific
//     prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.
*/
//=================================================================================================

#ifndef _BLAZE_MATH_TYPETRAITS_ISROWVECTOR_H_
#define _BLAZE_MATH_TYPETRAITS_ISROWVECTOR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <utility>
#include "../../math/expressions/Vector.h"
#include "../../math/TransposeFlag.h"
#include "../../util/FalseType.h"
#include "../../util/TrueType.h"
#include "../../util/typetraits/RemoveCV.h"


namespace blaze {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Auxiliary helper struct for the IsRowVector type trait.
// \ingroup math_type_traits
*/
template< typename T >
struct IsRowVectorHelper
{
 private:
   //**********************************************************************************************
   template< typename VT >
   static TrueType test( const Vector<VT,rowVector>& );

   static FalseType test( ... );
   //**********************************************************************************************

 public:
   //**********************************************************************************************
   using Type = decltype( test( std::declval< RemoveCV_<T> >() ) );
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Compile time check for row vector types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template argument is a row dense or sparse
// vector type (i.e. a vector whose transposition flag is set to blaze::rowVector). In case
// the type is a row vector type, the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using blaze::rowVector;
   using blaze::columnVector;

   blaze::IsRowVector< StaticVector<float,3U,rowVector> >::value         // Evaluates to 1
   blaze::IsRowVector< const DynamicVector<double,rowVector> >::Type     // Results in TrueType
   blaze::IsRowVector< volatile CompressedVector<int,rowVector> >        // Is derived from TrueType
   blaze::IsRowVector< StaticVector<float,3U,columnVector> >::value      // Evaluates to 0
   blaze::IsRowVector< const DynamicVector<double,columnVector> >::Type  // Results in FalseType
   blaze::IsRowVector< volatile CompressedVector<int,columnVector> >     // Is derived from FalseType
   \endcode
*/
template< typename T >
struct IsRowVector
   : public IsRowVectorHelper<T>::Type
{};
//*************************************************************************************************

} // namespace blaze

#endif
