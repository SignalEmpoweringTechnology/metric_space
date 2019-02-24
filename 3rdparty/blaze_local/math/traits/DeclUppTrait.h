//=================================================================================================
/*!
//  \file blaze/math/traits/DeclUppTrait.h
//  \brief Header file for the declupp trait
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

#ifndef _BLAZE_MATH_TRAITS_DECLUPPTRAIT_H_
#define _BLAZE_MATH_TRAITS_DECLUPPTRAIT_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../../math/adaptors/uppermatrix/BaseTemplate.h"
#include "../../math/typetraits/IsMatrix.h"
#include "../../util/InvalidType.h"
#include "../../util/mpl/If.h"
#include "../../util/mpl/Or.h"
#include "../../util/typetraits/Decay.h"
#include "../../util/typetraits/IsConst.h"
#include "../../util/typetraits/IsReference.h"
#include "../../util/typetraits/IsVolatile.h"


namespace blaze {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base template for the DeclUppTrait class.
// \ingroup math_traits
//
// \section declupptrait_general General
//
// The DeclUppTrait class template offers the possibility to select the resulting data type
// of a generic declupp() operation on the given type \a MT. DeclUppTrait defines the nested
// type \a Type, which represents the resulting data type of the declupp() operation. In case
// the given data type is not a dense or sparse matrix type, the resulting data type \a Type is
// set to \a INVALID_TYPE. Note that \a const and \a volatile qualifiers and reference modifiers
// are generally ignored.
//
//
// \section declupptrait_specializations Creating custom specializations
//
// Per default, DeclUppTrait supports all matrix types of the Blaze library (including views and
// adaptors). For all other data types it is possible to specialize the DeclUppTrait template. The
// following example shows the according specialization for the SymmetricMatrix class template:

   \code
   template< typename MT, bool SO, bool DF, bool NF >
   struct DeclUppTrait< SymmetricMatrix<MT,SO,DF,NF> >
   {
      using Type = DiagonalMatrix<MT>;
   };
   \endcode

// \n \section declupptrait_examples Examples
//
// The following example demonstrates the use of the DeclUppTrait template, where depending on
// the given matrix type the resulting type is selected:

   \code
   using blaze::DynamicMatrix;
   using blaze::StaticMatrix;
   using blaze::SymmetricMatrix;
   using blaze::DeclUppTrait;
   using blaze::rowMajor;
   using blaze::columnMajor;

   // Definition of the resulting type of a row-major dynamic matrix
   using MatrixType1  = DynamicMatrix<int,rowMajor>;
   using DeclUppType1 = typename DeclUppTrait<MatrixType1>::Type;

   // Definition of the resulting type of a symmetric column-major static matrix
   using MatrixType2  = SymmetricMatrix< StaticMatrix<int,3UL,3UL,columnMajor> >;
   using DeclUppType2 = typename DeclUppTrait<MatrixType2>::Type;
   \endcode
*/
template< typename MT >  // Type of the matrix
struct DeclUppTrait
{
 private:
   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   struct Failure { using Type = INVALID_TYPE; };
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   struct Result { using Type = UpperMatrix<MT>; };
   /*! \endcond */
   //**********************************************************************************************

 public:
   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   using Type = typename If_< Or< IsConst<MT>, IsVolatile<MT>, IsReference<MT> >
                            , DeclUppTrait< Decay_<MT> >
                            , If_< IsMatrix<MT>
                                 , Result
                                 , Failure > >::Type;
   /*! \endcond */
   //**********************************************************************************************
};
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the DeclUppTrait type trait.
// \ingroup math_traits
//
// The DeclUppTrait_ alias declaration provides a convenient shortcut to access the nested
// \a Type of the DeclUppTrait class template. For instance, given the matrix type \a MT the
// following two type definitions are identical:

   \code
   using Type1 = typename DeclUppTrait<MT>::Type;
   using Type2 = DeclUppTrait_<MT>;
   \endcode
*/
template< typename MT >  // Type of the matrix
using DeclUppTrait_ = typename DeclUppTrait<MT>::Type;
//*************************************************************************************************

} // namespace blaze

#endif
