//=================================================================================================
/*!
//  \file blaze/math/typetraits/IsStrictlyLower.h
//  \brief Header file for the IsStrictlyLower type trait
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

#ifndef _BLAZE_MATH_TYPETRAITS_ISSTRICTLYLOWER_H_
#define _BLAZE_MATH_TYPETRAITS_ISSTRICTLYLOWER_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../../util/FalseType.h"
#include "../../util/TrueType.h"


namespace blaze {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for strictly lower triangular matrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a strictly lower
// triangular matrix type (i.e. a matrix type that is guaranteed to be strictly lower
// triangular at compile time). In case the type is a strictly lower triangular matrix
// type, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using blaze::rowMajor;

   using StaticMatrixType     = blaze::StaticMatrix<double,3UL,3UL,rowMajor>;
   using DynamicMatrixType    = blaze::DynamicMatrix<float,rowMajor>;
   using CompressedMatrixType = blaze::CompressedMatrix<int,rowMajor>;

   using StrictlyLowerStaticType     = blaze::StrictlyLowerMatrix<StaticMatrixType>;
   using StrictlyLowerDynamicType    = blaze::StrictlyLowerMatrix<DynamicMatrixType>;
   using StrictlyLowerCompressedType = blaze::StrictlyLowerMatrix<CompressedMatrixType>;

   blaze::IsStrictlyLower< StrictlyLowerStaticType >::value        // Evaluates to 1
   blaze::IsStrictlyLower< const StrictlyLowerDynamicType >::Type  // Results in TrueType
   blaze::IsStrictlyLower< volatile StrictlyLowerCompressedType >  // Is derived from TrueType
   blaze::IsStrictlyLower< StaticMatrixType >::value               // Evaluates to 0
   blaze::IsStrictlyLower< const DynamicMatrixType >::Type         // Results in FalseType
   blaze::IsStrictlyLower< volatile CompressedMatrixType >         // Is derived from FalseType
   \endcode
*/
template< typename T >
struct IsStrictlyLower
   : public FalseType
{};
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization of the IsStrictlyLower type trait for const types.
// \ingroup math_type_traits
*/
template< typename T >
struct IsStrictlyLower< const T >
   : public IsStrictlyLower<T>
{};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization of the IsStrictlyLower type trait for volatile types.
// \ingroup math_type_traits
*/
template< typename T >
struct IsStrictlyLower< volatile T >
   : public IsStrictlyLower<T>
{};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization of the IsStrictlyLower type trait for cv qualified types.
// \ingroup math_type_traits
*/
template< typename T >
struct IsStrictlyLower< const volatile T >
   : public IsStrictlyLower<T>
{};
/*! \endcond */
//*************************************************************************************************

} // namespace blaze

#endif
