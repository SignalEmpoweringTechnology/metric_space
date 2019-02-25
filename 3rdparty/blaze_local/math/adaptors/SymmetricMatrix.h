//=================================================================================================
/*!
//  \file blaze/math/adaptors/SymmetricMatrix.h
//  \brief Header file for the implementation of a symmetric matrix adaptor
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

#ifndef _BLAZE_MATH_ADAPTORS_SYMMETRICMATRIX_H_
#define _BLAZE_MATH_ADAPTORS_SYMMETRICMATRIX_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../../math/Aliases.h"
#include "../../math/adaptors/symmetricmatrix/BaseTemplate.h"
#include "../../math/adaptors/symmetricmatrix/DenseNonNumeric.h"
#include "../../math/adaptors/symmetricmatrix/DenseNumeric.h"
#include "../../math/adaptors/symmetricmatrix/SparseNonNumeric.h"
#include "../../math/adaptors/symmetricmatrix/SparseNumeric.h"
#include "../../math/constraints/BLASCompatible.h"
#include "../../math/constraints/RequiresEvaluation.h"
#include "../../math/Exception.h"
#include "../../math/InversionFlag.h"
#include "../../math/Forward.h"
#include "../../math/shims/IsDefault.h"
#include "../../math/shims/IsDivisor.h"
#include "../../math/traits/AddTrait.h"
#include "../../math/traits/BandTrait.h"
#include "../../math/traits/BinaryMapTrait.h"
#include "../../math/traits/ColumnsTrait.h"
#include "../../math/traits/ColumnTrait.h"
#include "../../math/traits/DeclDiagTrait.h"
#include "../../math/traits/DeclHermTrait.h"
#include "../../math/traits/DeclLowTrait.h"
#include "../../math/traits/DeclSymTrait.h"
#include "../../math/traits/DeclUppTrait.h"
#include "../../math/traits/DivTrait.h"
#include "../../math/traits/MultTrait.h"
#include "../../math/traits/RowsTrait.h"
#include "../../math/traits/RowTrait.h"
#include "../../math/traits/SchurTrait.h"
#include "../../math/traits/SubmatrixTrait.h"
#include "../../math/traits/SubTrait.h"
#include "../../math/traits/UnaryMapTrait.h"
#include "../../math/typetraits/HasConstDataAccess.h"
#include "../../math/typetraits/HighType.h"
#include "../../math/typetraits/IsAdaptor.h"
#include "../../math/typetraits/IsAligned.h"
#include "../../math/typetraits/IsContiguous.h"
#include "../../math/typetraits/IsHermitian.h"
#include "../../math/typetraits/IsPadded.h"
#include "../../math/typetraits/IsResizable.h"
#include "../../math/typetraits/IsRestricted.h"
#include "../../math/typetraits/IsShrinkable.h"
#include "../../math/typetraits/IsSquare.h"
#include "../../math/typetraits/IsSymmetric.h"
#include "../../math/typetraits/LowType.h"
#include "../../math/typetraits/RemoveAdaptor.h"
#include "../../math/typetraits/Size.h"
#include "../../util/algorithms/Min.h"
#include "../../util/Assert.h"
#include "../../util/EnableIf.h"
#include "../../util/TrueType.h"
#include "../../util/typetraits/IsBuiltin.h"
#include "../../util/typetraits/IsNumeric.h"
#include "../../util/Unused.h"


namespace blaze {

//=================================================================================================
//
//  SYMMETRICMATRIX OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\name SymmetricMatrix operators */
//@{
template< typename MT, bool SO, bool DF, bool NF >
inline void reset( SymmetricMatrix<MT,SO,DF,NF>& m );

template< typename MT, bool SO, bool DF, bool NF >
inline void reset( SymmetricMatrix<MT,SO,DF,NF>& m, size_t i );

template< typename MT, bool SO, bool DF, bool NF >
inline void clear( SymmetricMatrix<MT,SO,DF,NF>& m );

template< bool RF, typename MT, bool SO, bool DF, bool NF >
inline bool isDefault( const SymmetricMatrix<MT,SO,DF,NF>& m );

template< typename MT, bool SO, bool DF, bool NF >
inline bool isIntact( const SymmetricMatrix<MT,SO,DF,NF>& m );

template< typename MT, bool SO, bool DF, bool NF >
inline void swap( SymmetricMatrix<MT,SO,DF,NF>& a, SymmetricMatrix<MT,SO,DF,NF>& b ) noexcept;
//@}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Resetting the given symmetric matrix.
// \ingroup symmetric_matrix
//
// \param m The symmetric matrix to be resetted.
// \return void
*/
template< typename MT  // Type of the adapted matrix
        , bool SO      // Storage order of the adapted matrix
        , bool DF      // Density flag
        , bool NF >    // Numeric flag
inline void reset( SymmetricMatrix<MT,SO,DF,NF>& m )
{
   m.reset();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Resetting the specified row/column of the given symmetric matrix.
// \ingroup symmetric_matrix
//
// \param m The symmetric matrix to be resetted.
// \param i The index of the row/column to be resetted.
// \return void
//
// This function resets the values in the specified row/column of the given symmetric matrix to
// their default value. In case the given matrix is a \a rowMajor matrix the function resets the
// values in row \a i, if it is a \a columnMajor matrix the function resets the values in column
// \a i. Note that the capacity of the row/column remains unchanged.
*/
template< typename MT  // Type of the adapted matrix
        , bool SO      // Storage order of the adapted matrix
        , bool DF      // Density flag
        , bool NF >    // Numeric flag
inline void reset( SymmetricMatrix<MT,SO,DF,NF>& m, size_t i )
{
   m.reset( i );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Clearing the given symmetric matrix.
// \ingroup symmetric_matrix
//
// \param m The symmetric matrix to be cleared.
// \return void
*/
template< typename MT  // Type of the adapted matrix
        , bool SO      // Storage order of the adapted matrix
        , bool DF      // Density flag
        , bool NF >    // Numeric flag
inline void clear( SymmetricMatrix<MT,SO,DF,NF>& m )
{
   m.clear();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the given symmetric matrix is in default state.
// \ingroup symmetric_matrix
//
// \param m The symmetric matrix to be tested for its default state.
// \return \a true in case the given matrix is component-wise zero, \a false otherwise.
//
// This function checks whether the matrix is in default state. For instance, in case the
// matrix is instantiated for a built-in integral or floating point data type, the function
// returns \a true in case all matrix elements are 0 and \a false in case any matrix element
// is not 0. The following example demonstrates the use of the \a isDefault function:

   \code
   blaze::SymmetricMatrix<int> A;
   // ... Resizing and initialization
   if( isDefault( A ) ) { ... }
   \endcode

// Optionally, it is possible to switch between strict semantics (blaze::strict) and relaxed
// semantics (blaze::relaxed):

   \code
   if( isDefault<relaxed>( A ) ) { ... }
   \endcode
*/
template< bool RF      // Relaxation flag
        , typename MT  // Type of the adapted matrix
        , bool SO      // Storage order of the adapted matrix
        , bool DF      // Density flag
        , bool NF >    // Numeric flag
inline bool isDefault( const SymmetricMatrix<MT,SO,DF,NF>& m )
{
   return isDefault<RF>( m.matrix_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the invariants of the given symmetric matrix are intact.
// \ingroup symmetric_matrix
//
// \param m The symmetric matrix to be tested.
// \return \a true in case the given matrix's invariants are intact, \a false otherwise.
//
// This function checks whether the invariants of the symmetric matrix are intact, i.e. if its
// state is valid. In case the invariants are intact, the function returns \a true, else it
// will return \a false. The following example demonstrates the use of the \a isIntact()
// function:

   \code
   using blaze::DynamicMatrix;
   using blaze::SymmetricMatrix;

   SymmetricMatrix< DynamicMatrix<int> > A;
   // ... Resizing and initialization
   if( isIntact( A ) ) { ... }
   \endcode
*/
template< typename MT  // Type of the adapted matrix
        , bool SO      // Storage order of the adapted matrix
        , bool DF      // Density flag
        , bool NF >    // Numeric flag
inline bool isIntact( const SymmetricMatrix<MT,SO,DF,NF>& m )
{
   return m.isIntact();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Swapping the contents of two matrices.
// \ingroup symmetric_matrix
//
// \param a The first matrix to be swapped.
// \param b The second matrix to be swapped.
// \return void
*/
template< typename MT  // Type of the adapted matrix
        , bool SO      // Storage order of the adapted matrix
        , bool DF      // Density flag
        , bool NF >    // Numeric flag
inline void swap( SymmetricMatrix<MT,SO,DF,NF>& a, SymmetricMatrix<MT,SO,DF,NF>& b ) noexcept
{
   a.swap( b );
}
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief In-place inversion of the given symmetric dense matrix.
// \ingroup symmetric_matrix
//
// \param m The symmetric dense matrix to be inverted.
// \return void
// \exception std::invalid_argument Inversion of singular matrix failed.
//
// This function inverts the given symmetric dense matrix by means of the specified matrix
// inversion algorithm \c IF. The The inversion fails if the given matrix is singular and not
// invertible. In this case a \a std::invalid_argument exception is thrown.
//
// \note The matrix inversion can only be used for dense matrices with \c float, \c double,
// \c complex<float> or \c complex<double> element type. The attempt to call the function with
// matrices of any other element type results in a compile time error!
//
// \note This function can only be used if a fitting LAPACK library is available and linked to
// the executable. Otherwise a linker error will be created.
//
// \note This function does only provide the basic exception safety guarantee, i.e. in case of an
// exception \a m may already have been modified.
*/
template< InversionFlag IF  // Inversion algorithm
        , typename MT       // Type of the dense matrix
        , bool SO >         // Storage order of the dense matrix
inline void invert( SymmetricMatrix<MT,SO,true,true>& m )
{
   BLAZE_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE( ElementType_<MT> );

   if( IF == asUniLower || IF == asUniUpper ) {
      BLAZE_INTERNAL_ASSERT( isIdentity( m ), "Violation of preconditions detected" );
      return;
   }

   constexpr InversionFlag flag( ( IF == byLU || IF == byLDLT || IF == byLDLH ||
                                   IF == asGeneral || IF == asSymmetric || IF == asHermitian )
                                 ? ( byLDLT )
                                 : ( ( IF == byLLH )
                                     ?( byLLH )
                                     :( asDiagonal ) ) );

   MT tmp( m.matrix_ );
   invert<flag>( tmp );
   m.matrix_ = std::move( tmp );

   BLAZE_INTERNAL_ASSERT( isIntact( m ), "Broken invariant detected" );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Predict invariant violations by the assignment of a matrix to a symmetric matrix.
// \ingroup symmetric_matrix
//
// \param lhs The target left-hand side symmetric matrix.
// \param rhs The right-hand side matrix to be assigned.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT1  // Type of the adapted matrix
        , bool SO1      // Storage order of the adapted matrix
        , bool DF       // Density flag
        , bool NF       // Numeric flag
        , typename MT2  // Type of the right-hand side matrix
        , bool SO2 >    // Storage order of the right-hand side matrix
inline bool tryAssign( const SymmetricMatrix<MT1,SO1,DF,NF>& lhs,
                       const Matrix<MT2,SO2>& rhs, size_t row, size_t column )
{
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( MT2 );

   BLAZE_INTERNAL_ASSERT( row <= lhs.rows(), "Invalid row access index" );
   BLAZE_INTERNAL_ASSERT( column <= lhs.columns(), "Invalid column access index" );
   BLAZE_INTERNAL_ASSERT( (~rhs).rows() <= lhs.rows() - row, "Invalid number of rows" );
   BLAZE_INTERNAL_ASSERT( (~rhs).columns() <= lhs.columns() - column, "Invalid number of columns" );

   UNUSED_PARAMETER( lhs );

   const size_t M( (~rhs).rows()    );
   const size_t N( (~rhs).columns() );

   if( ( row + M <= column ) || ( column + N <= row ) )
      return true;

   const bool   lower( row > column );
   const size_t size ( min( row + M, column + N ) - ( lower ? row : column ) );

   if( size < 2UL )
      return true;

   const size_t subrow( lower ? 0UL : column - row );
   const size_t subcol( lower ? row - column : 0UL );

   return isSymmetric( submatrix( ~rhs, subrow, subcol, size, size ) );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Predict invariant violations by the addition assignment of a matrix to a symmetric matrix.
// \ingroup symmetric_matrix
//
// \param lhs The target left-hand side symmetric matrix.
// \param rhs The right-hand side matrix to be added.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT1  // Type of the adapted matrix
        , bool SO1      // Storage order of the adapted matrix
        , bool DF       // Density flag
        , bool NF       // Numeric flag
        , typename MT2  // Type of the right-hand side matrix
        , bool SO2 >    // Storage order of the right-hand side matrix
inline bool tryAddAssign( const SymmetricMatrix<MT1,SO1,DF,NF>& lhs,
                          const Matrix<MT2,SO2>& rhs, size_t row, size_t column )
{
   return tryAssign( lhs, ~rhs, row, column );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Predict invariant violations by the subtraction assignment of a matrix to a symmetric
//        matrix.
// \ingroup symmetric_matrix
//
// \param lhs The target left-hand side symmetric matrix.
// \param rhs The right-hand side matrix to be subtracted.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT1  // Type of the adapted matrix
        , bool SO1      // Storage order of the adapted matrix
        , bool DF       // Density flag
        , bool NF       // Numeric flag
        , typename MT2  // Type of the right-hand side matrix
        , bool SO2 >    // Storage order of the right-hand side matrix
inline bool trySubAssign( const SymmetricMatrix<MT1,SO1,DF,NF>& lhs,
                          const Matrix<MT2,SO2>& rhs, size_t row, size_t column )
{
   return tryAssign( lhs, ~rhs, row, column );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Predict invariant violations by the Schur product assignment of a matrix to a symmetric
//        matrix.
// \ingroup symmetric_matrix
//
// \param lhs The target left-hand side symmetric matrix.
// \param rhs The right-hand side matrix for the Schur product.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename MT1  // Type of the adapted matrix
        , bool SO1      // Storage order of the adapted matrix
        , bool DF       // Density flag
        , bool NF       // Numeric flag
        , typename MT2  // Type of the right-hand side matrix
        , bool SO2 >    // Storage order of the right-hand side matrix
inline bool trySchurAssign( const SymmetricMatrix<MT1,SO1,DF,NF>& lhs,
                            const Matrix<MT2,SO2>& rhs, size_t row, size_t column )
{
   return tryAssign( lhs, ~rhs, row, column );
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  SIZE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct Size< SymmetricMatrix<MT,SO,DF,NF>, 0UL >
   : public Size<MT,0UL>
{};

template< typename MT, bool SO, bool DF, bool NF >
struct Size< SymmetricMatrix<MT,SO,DF,NF>, 1UL >
   : public Size<MT,1UL>
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISSQUARE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct IsSquare< SymmetricMatrix<MT,SO,DF,NF> >
   : public TrueType
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISSYMMETRIC SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct IsSymmetric< SymmetricMatrix<MT,SO,DF,NF> >
   : public TrueType
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISHERMITIAN SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct IsHermitian< SymmetricMatrix<MT,SO,DF,NF> >
   : public IsBuiltin< ElementType_<MT> >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISADAPTOR SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct IsAdaptor< SymmetricMatrix<MT,SO,DF,NF> >
   : public TrueType
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISRESTRICTED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct IsRestricted< SymmetricMatrix<MT,SO,DF,NF> >
   : public TrueType
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  HASCONSTDATAACCESS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool NF >
struct HasConstDataAccess< SymmetricMatrix<MT,SO,true,NF> >
   : public TrueType
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISALIGNED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct IsAligned< SymmetricMatrix<MT,SO,DF,NF> >
   : public IsAligned<MT>
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISCONTIGUOUS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct IsContiguous< SymmetricMatrix<MT,SO,DF,NF> >
   : public IsContiguous<MT>
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISPADDED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct IsPadded< SymmetricMatrix<MT,SO,DF,NF> >
   : public IsPadded<MT>
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISRESIZABLE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct IsResizable< SymmetricMatrix<MT,SO,DF,NF> >
   : public IsResizable<MT>
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISSHRINKABLE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct IsShrinkable< SymmetricMatrix<MT,SO,DF,NF> >
   : public IsShrinkable<MT>
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  REMOVEADAPTOR SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct RemoveAdaptor< SymmetricMatrix<MT,SO,DF,NF> >
{
   using Type = MT;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ADDTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO1, bool DF, bool NF, typename T, size_t M, size_t N, bool SO2 >
struct AddTrait< SymmetricMatrix<MT,SO1,DF,NF>, StaticMatrix<T,M,N,SO2> >
{
   using Type = AddTrait_< MT, StaticMatrix<T,M,N,SO2> >;
};

template< typename T, size_t M, size_t N, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct AddTrait< StaticMatrix<T,M,N,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = AddTrait_< StaticMatrix<T,M,N,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, size_t M, size_t N, bool SO2 >
struct AddTrait< SymmetricMatrix<MT,SO1,DF,NF>, HybridMatrix<T,M,N,SO2> >
{
   using Type = AddTrait_< MT, HybridMatrix<T,M,N,SO2> >;
};

template< typename T, size_t M, size_t N, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct AddTrait< HybridMatrix<T,M,N,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = AddTrait_< HybridMatrix<T,M,N,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool SO2 >
struct AddTrait< SymmetricMatrix<MT,SO1,DF,NF>, DynamicMatrix<T,SO2> >
{
   using Type = AddTrait_< MT, DynamicMatrix<T,SO2> >;
};

template< typename T, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct AddTrait< DynamicMatrix<T,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = AddTrait_< DynamicMatrix<T,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool AF, bool PF, bool SO2 >
struct AddTrait< SymmetricMatrix<MT,SO1,DF,NF>, CustomMatrix<T,AF,PF,SO2> >
{
   using Type = AddTrait_< MT, CustomMatrix<T,AF,PF,SO2> >;
};

template< typename T, bool AF, bool PF, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct AddTrait< CustomMatrix<T,AF,PF,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = AddTrait_< CustomMatrix<T,AF,PF,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool SO2 >
struct AddTrait< SymmetricMatrix<MT,SO1,DF,NF>, CompressedMatrix<T,SO2> >
{
   using Type = AddTrait_< MT, CompressedMatrix<T,SO2> >;
};

template< typename T, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct AddTrait< CompressedMatrix<T,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = AddTrait_< CompressedMatrix<T,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool SO2 >
struct AddTrait< SymmetricMatrix<MT,SO1,DF,NF>, IdentityMatrix<T,SO2> >
{
   using Type = SymmetricMatrix< AddTrait_< MT, IdentityMatrix<T,SO2> > >;
};

template< typename T, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct AddTrait< IdentityMatrix<T,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = SymmetricMatrix< AddTrait_< IdentityMatrix<T,SO1>, MT > >;
};

template< typename MT1, bool SO1, bool DF1, bool NF1, typename MT2, bool SO2, bool DF2, bool NF2 >
struct AddTrait< SymmetricMatrix<MT1,SO1,DF1,NF1>, SymmetricMatrix<MT2,SO2,DF2,NF2> >
{
   using Type = SymmetricMatrix< AddTrait_<MT1,MT2> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  SUBTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO1, bool DF, bool NF, typename T, size_t M, size_t N, bool SO2 >
struct SubTrait< SymmetricMatrix<MT,SO1,DF,NF>, StaticMatrix<T,M,N,SO2> >
{
   using Type = SubTrait_< MT, StaticMatrix<T,M,N,SO2> >;
};

template< typename T, size_t M, size_t N, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct SubTrait< StaticMatrix<T,M,N,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = SubTrait_< StaticMatrix<T,M,N,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, size_t M, size_t N, bool SO2 >
struct SubTrait< SymmetricMatrix<MT,SO1,DF,NF>, HybridMatrix<T,M,N,SO2> >
{
   using Type = SubTrait_< MT, HybridMatrix<T,M,N,SO2> >;
};

template< typename T, size_t M, size_t N, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct SubTrait< HybridMatrix<T,M,N,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = SubTrait_< HybridMatrix<T,M,N,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool SO2 >
struct SubTrait< SymmetricMatrix<MT,SO1,DF,NF>, DynamicMatrix<T,SO2> >
{
   using Type = SubTrait_< MT, DynamicMatrix<T,SO2> >;
};

template< typename T, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct SubTrait< DynamicMatrix<T,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = SubTrait_< DynamicMatrix<T,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool AF, bool PF, bool SO2 >
struct SubTrait< SymmetricMatrix<MT,SO1,DF,NF>, CustomMatrix<T,AF,PF,SO2> >
{
   using Type = SubTrait_< MT, CustomMatrix<T,AF,PF,SO2> >;
};

template< typename T, bool AF, bool PF, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct SubTrait< CustomMatrix<T,AF,PF,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = SubTrait_< CustomMatrix<T,AF,PF,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool SO2 >
struct SubTrait< SymmetricMatrix<MT,SO1,DF,NF>, CompressedMatrix<T,SO2> >
{
   using Type = SubTrait_< MT, CompressedMatrix<T,SO2> >;
};

template< typename T, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct SubTrait< CompressedMatrix<T,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = SubTrait_< CompressedMatrix<T,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool SO2 >
struct SubTrait< SymmetricMatrix<MT,SO1,DF,NF>, IdentityMatrix<T,SO2> >
{
   using Type = SymmetricMatrix< SubTrait_< MT, IdentityMatrix<T,SO2> > >;
};

template< typename T, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct SubTrait< IdentityMatrix<T,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = SymmetricMatrix< SubTrait_< IdentityMatrix<T,SO1>, MT > >;
};

template< typename MT1, bool SO1, bool DF1, bool NF1, typename MT2, bool SO2, bool DF2, bool NF2 >
struct SubTrait< SymmetricMatrix<MT1,SO1,DF1,NF1>, SymmetricMatrix<MT2,SO2,DF2,NF2> >
{
   using Type = SymmetricMatrix< SubTrait_<MT1,MT2> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  SCHURTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO1, bool DF, bool NF, typename T, size_t M, size_t N, bool SO2 >
struct SchurTrait< SymmetricMatrix<MT,SO1,DF,NF>, StaticMatrix<T,M,N,SO2> >
{
   using Type = SchurTrait_< MT, StaticMatrix<T,M,N,SO2> >;
};

template< typename T, size_t M, size_t N, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct SchurTrait< StaticMatrix<T,M,N,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = SchurTrait_< StaticMatrix<T,M,N,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, size_t M, size_t N, bool SO2 >
struct SchurTrait< SymmetricMatrix<MT,SO1,DF,NF>, HybridMatrix<T,M,N,SO2> >
{
   using Type = SchurTrait_< MT, HybridMatrix<T,M,N,SO2> >;
};

template< typename T, size_t M, size_t N, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct SchurTrait< HybridMatrix<T,M,N,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = SchurTrait_< HybridMatrix<T,M,N,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool SO2 >
struct SchurTrait< SymmetricMatrix<MT,SO1,DF,NF>, DynamicMatrix<T,SO2> >
{
   using Type = SchurTrait_< MT, DynamicMatrix<T,SO2> >;
};

template< typename T, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct SchurTrait< DynamicMatrix<T,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = SchurTrait_< DynamicMatrix<T,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool AF, bool PF, bool SO2 >
struct SchurTrait< SymmetricMatrix<MT,SO1,DF,NF>, CustomMatrix<T,AF,PF,SO2> >
{
   using Type = SchurTrait_< MT, CustomMatrix<T,AF,PF,SO2> >;
};

template< typename T, bool AF, bool PF, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct SchurTrait< CustomMatrix<T,AF,PF,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = SchurTrait_< CustomMatrix<T,AF,PF,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool SO2 >
struct SchurTrait< SymmetricMatrix<MT,SO1,DF,NF>, CompressedMatrix<T,SO2> >
{
   using Type = SchurTrait_< MT, CompressedMatrix<T,SO2> >;
};

template< typename T, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct SchurTrait< CompressedMatrix<T,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = SchurTrait_< CompressedMatrix<T,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool SO2 >
struct SchurTrait< SymmetricMatrix<MT,SO1,DF,NF>, IdentityMatrix<T,SO2> >
{
   using Type = DiagonalMatrix< SchurTrait_< MT, IdentityMatrix<T,SO2> > >;
};

template< typename T, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct SchurTrait< IdentityMatrix<T,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = DiagonalMatrix< SchurTrait_< IdentityMatrix<T,SO1>, MT > >;
};

template< typename MT1, bool SO1, bool DF1, bool NF1, typename MT2, bool SO2, bool DF2, bool NF2 >
struct SchurTrait< SymmetricMatrix<MT1,SO1,DF1,NF1>, SymmetricMatrix<MT2,SO2,DF2,NF2> >
{
   using Type = SymmetricMatrix< SchurTrait_<MT1,MT2> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  MULTTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF, typename T >
struct MultTrait< SymmetricMatrix<MT,SO,DF,NF>, T, EnableIf_< IsNumeric<T> > >
{
   using Type = SymmetricMatrix< MultTrait_<MT,T> >;
};

template< typename T, typename MT, bool SO, bool DF, bool NF >
struct MultTrait< T, SymmetricMatrix<MT,SO,DF,NF>, EnableIf_< IsNumeric<T> > >
{
   using Type = SymmetricMatrix< MultTrait_<T,MT> >;
};

template< typename MT, bool SO, bool DF, bool NF, typename T, size_t N >
struct MultTrait< SymmetricMatrix<MT,SO,DF,NF>, StaticVector<T,N,false> >
{
   using Type = MultTrait_< MT, StaticVector<T,N,false> >;
};

template< typename T, size_t N, typename MT, bool SO, bool DF, bool NF >
struct MultTrait< StaticVector<T,N,true>, SymmetricMatrix<MT,SO,DF,NF> >
{
   using Type = MultTrait_< StaticVector<T,N,true>, MT >;
};

template< typename MT, bool SO, bool DF, bool NF, typename T, size_t N >
struct MultTrait< SymmetricMatrix<MT,SO,DF,NF>, HybridVector<T,N,false> >
{
   using Type = MultTrait_< MT, HybridVector<T,N,false> >;
};

template< typename T, size_t N, typename MT, bool SO, bool DF, bool NF >
struct MultTrait< HybridVector<T,N,true>, SymmetricMatrix<MT,SO,DF,NF> >
{
   using Type = MultTrait_< HybridVector<T,N,true>, MT >;
};

template< typename MT, bool SO, bool DF, bool NF, typename T >
struct MultTrait< SymmetricMatrix<MT,SO,DF,NF>, DynamicVector<T,false> >
{
   using Type = MultTrait_< MT, DynamicVector<T,false> >;
};

template< typename T, typename MT, bool SO, bool DF, bool NF >
struct MultTrait< DynamicVector<T,true>, SymmetricMatrix<MT,SO,DF,NF> >
{
   using Type = MultTrait_< DynamicVector<T,true>, MT >;
};

template< typename MT, bool SO, bool DF, bool NF, typename T, bool AF, bool PF >
struct MultTrait< SymmetricMatrix<MT,SO,DF,NF>, CustomVector<T,AF,PF,false> >
{
   using Type = MultTrait_< MT, CustomVector<T,AF,PF,false> >;
};

template< typename T, bool AF, bool PF, typename MT, bool SO, bool DF, bool NF >
struct MultTrait< CustomVector<T,AF,PF,true>, SymmetricMatrix<MT,SO,DF,NF> >
{
   using Type = MultTrait_< CustomVector<T,AF,PF,true>, MT >;
};

template< typename MT, bool SO, bool DF, bool NF, typename T >
struct MultTrait< SymmetricMatrix<MT,SO,DF,NF>, CompressedVector<T,false> >
{
   using Type = MultTrait_< MT, CompressedVector<T,false> >;
};

template< typename T, typename MT, bool SO, bool DF, bool NF >
struct MultTrait< CompressedVector<T,true>, SymmetricMatrix<MT,SO,DF,NF> >
{
   using Type = MultTrait_< CompressedVector<T,true>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, size_t M, size_t N, bool SO2 >
struct MultTrait< SymmetricMatrix<MT,SO1,DF,NF>, StaticMatrix<T,M,N,SO2> >
{
   using Type = MultTrait_< MT, StaticMatrix<T,M,N,SO2> >;
};

template< typename T, size_t M, size_t N, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct MultTrait< StaticMatrix<T,M,N,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = MultTrait_< StaticMatrix<T,M,N,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, size_t M, size_t N, bool SO2 >
struct MultTrait< SymmetricMatrix<MT,SO1,DF,NF>, HybridMatrix<T,M,N,SO2> >
{
   using Type = MultTrait_< MT, HybridMatrix<T,M,N,SO2> >;
};

template< typename T, size_t M, size_t N, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct MultTrait< HybridMatrix<T,M,N,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = MultTrait_< HybridMatrix<T,M,N,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool SO2 >
struct MultTrait< SymmetricMatrix<MT,SO1,DF,NF>, DynamicMatrix<T,SO2> >
{
   using Type = MultTrait_< MT, DynamicMatrix<T,SO2> >;
};

template< typename T, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct MultTrait< DynamicMatrix<T,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = MultTrait_< DynamicMatrix<T,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool AF, bool PF, bool SO2 >
struct MultTrait< SymmetricMatrix<MT,SO1,DF,NF>, CustomMatrix<T,AF,PF,SO2> >
{
   using Type = MultTrait_< MT, CustomMatrix<T,AF,PF,SO2> >;
};

template< typename T, bool AF, bool PF, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct MultTrait< CustomMatrix<T,AF,PF,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = MultTrait_< DynamicMatrix<T,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool SO2 >
struct MultTrait< SymmetricMatrix<MT,SO1,DF,NF>, CompressedMatrix<T,SO2> >
{
   using Type = MultTrait_< MT, CompressedMatrix<T,SO2> >;
};

template< typename T, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct MultTrait< CompressedMatrix<T,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = MultTrait_< CompressedMatrix<T,SO1>, MT >;
};

template< typename MT, bool SO1, bool DF, bool NF, typename T, bool SO2 >
struct MultTrait< SymmetricMatrix<MT,SO1,DF,NF>, IdentityMatrix<T,SO2> >
{
   using Type = SymmetricMatrix< MultTrait_< MT, IdentityMatrix<T,SO2> > >;
};

template< typename T, bool SO1, typename MT, bool SO2, bool DF, bool NF >
struct MultTrait< IdentityMatrix<T,SO1>, SymmetricMatrix<MT,SO2,DF,NF> >
{
   using Type = SymmetricMatrix< MultTrait_< IdentityMatrix<T,SO1>, MT > >;
};

template< typename MT1, bool SO1, bool DF1, bool NF1, typename MT2, bool SO2, bool DF2, bool NF2 >
struct MultTrait< SymmetricMatrix<MT1,SO1,DF1,NF1>, SymmetricMatrix<MT2,SO2,DF2,NF2> >
{
   using Type = MultTrait_<MT1,MT2>;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DIVTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF, typename T >
struct DivTrait< SymmetricMatrix<MT,SO,DF,NF>, T, EnableIf_< IsNumeric<T> > >
{
   using Type = SymmetricMatrix< DivTrait_<MT,T> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  UNARYMAPTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Abs >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Abs> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Floor >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Floor> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Ceil >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Ceil> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Trunc >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Trunc> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Round >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Round> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Conj >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Conj> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Real >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Real> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Imag >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Imag> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Sqrt >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Sqrt> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, InvSqrt >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,InvSqrt> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Cbrt >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Cbrt> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, InvCbrt >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,InvCbrt> >;
};

template< typename MT, bool SO, bool DF, bool NF, typename ET >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, UnaryPow<ET> >
{
   using Type = SymmetricMatrix< UnaryMapTrait_< MT, UnaryPow<ET> > >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Exp >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Exp> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Log >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Log> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Log10 >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Log10> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Sin >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Sin> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Asin >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Asin> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Sinh >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Sinh> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Asinh >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Asinh> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Cos >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Cos> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Acos >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Acos> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Cosh >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Cosh> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Acosh >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Acosh> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Tan >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Tan> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Atan >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Atan> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Tanh >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Tanh> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Atanh >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Atanh> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Erf >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Erf> >;
};

template< typename MT, bool SO, bool DF, bool NF >
struct UnaryMapTrait< SymmetricMatrix<MT,SO,DF,NF>, Erfc >
{
   using Type = SymmetricMatrix< UnaryMapTrait_<MT,Erfc> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  BINARYMAPTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, bool SO1, bool DF1, bool NF1, typename MT2, bool SO2, bool DF2, bool NF2 >
struct BinaryMapTrait< SymmetricMatrix<MT1,SO1,DF1,NF1>, SymmetricMatrix<MT2,SO2,DF2,NF2>, Min >
{
   using Type = SymmetricMatrix< BinaryMapTrait_<MT1,MT2,Min> >;
};

template< typename MT1, bool SO1, bool DF1, bool NF1, typename MT2, bool SO2, bool DF2, bool NF2 >
struct BinaryMapTrait< SymmetricMatrix<MT1,SO1,DF1,NF1>, SymmetricMatrix<MT2,SO2,DF2,NF2>, Max >
{
   using Type = SymmetricMatrix< BinaryMapTrait_<MT1,MT2,Max> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DECLSYMTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct DeclSymTrait< SymmetricMatrix<MT,SO,DF,NF> >
{
   using Type = SymmetricMatrix<MT,SO,DF,NF>;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DECLHERMTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct DeclHermTrait< SymmetricMatrix<MT,SO,DF,NF> >
{
   using Type = HermitianMatrix<MT>;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DECLLOWTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct DeclLowTrait< SymmetricMatrix<MT,SO,DF,NF> >
{
   using Type = DiagonalMatrix<MT>;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DECLUPPTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct DeclUppTrait< SymmetricMatrix<MT,SO,DF,NF> >
{
   using Type = DiagonalMatrix<MT>;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DECLDIAGTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF >
struct DeclDiagTrait< SymmetricMatrix<MT,SO,DF,NF> >
{
   using Type = DiagonalMatrix<MT>;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  HIGHTYPE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, bool SO1, bool DF1, bool NF1, typename MT2, bool SO2, bool DF2, bool NF2 >
struct HighType< SymmetricMatrix<MT1,SO1,DF1,NF1>, SymmetricMatrix<MT2,SO2,DF2,NF2> >
{
   using Type = SymmetricMatrix< typename HighType<MT1,MT2>::Type >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  LOWTYPE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, bool SO1, bool DF1, bool NF1, typename MT2, bool SO2, bool DF2, bool NF2 >
struct LowType< SymmetricMatrix<MT1,SO1,DF1,NF1>, SymmetricMatrix<MT2,SO2,DF2,NF2> >
{
   using Type = SymmetricMatrix< typename LowType<MT1,MT2>::Type >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  SUBMATRIXTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF, size_t... CSAs >
struct SubmatrixTrait< SymmetricMatrix<MT,SO,DF,NF>, CSAs... >
{
   using Type = SubmatrixTrait_<MT,CSAs...>;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ROWTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF, size_t... CRAs >
struct RowTrait< SymmetricMatrix<MT,SO,DF,NF>, CRAs... >
{
   using Type = RowTrait_<MT,CRAs...>;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ROWSTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF, size_t... CRAs >
struct RowsTrait< SymmetricMatrix<MT,SO,DF,NF>, CRAs... >
{
   using Type = RowsTrait_<MT,CRAs...>;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  COLUMNTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF, size_t... CCAs >
struct ColumnTrait< SymmetricMatrix<MT,SO,DF,NF>, CCAs... >
{
   using Type = ColumnTrait_<MT,CCAs...>;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  COLUMNSTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF, size_t... CCAs >
struct ColumnsTrait< SymmetricMatrix<MT,SO,DF,NF>, CCAs... >
{
   using Type = ColumnsTrait_<MT,CCAs...>;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  BANDTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, bool SO, bool DF, bool NF, ptrdiff_t... CBAs >
struct BandTrait< SymmetricMatrix<MT,SO,DF,NF>, CBAs... >
{
   using Type = BandTrait_<MT,CBAs...>;
};
/*! \endcond */
//*************************************************************************************************

} // namespace blaze

#endif