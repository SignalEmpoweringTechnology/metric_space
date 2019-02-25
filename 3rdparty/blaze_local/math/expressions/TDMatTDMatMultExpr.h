//=================================================================================================
/*!
//  \file blaze/math/expressions/TDMatTDMatMultExpr.h
//  \brief Header file for the transpose dense matrix/transpose dense matrix multiplication expression
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

#ifndef _BLAZE_MATH_EXPRESSIONS_TDMATTDMATMULTEXPR_H_
#define _BLAZE_MATH_EXPRESSIONS_TDMATTDMATMULTEXPR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../../math/blas/gemm.h"
#include "../../math/blas/trmm.h"
#include "../../math/Aliases.h"
#include "../../math/constraints/ColumnMajorMatrix.h"
#include "../../math/constraints/DenseMatrix.h"
#include "../../math/constraints/MatMatMultExpr.h"
#include "../../math/constraints/RequiresEvaluation.h"
#include "../../math/constraints/RowMajorMatrix.h"
#include "../../math/constraints/StorageOrder.h"
#include "../../math/constraints/Symmetric.h"
#include "../../math/dense/MMM.h"
#include "../../math/Exception.h"
#include "../../math/expressions/Computation.h"
#include "../../math/expressions/DenseMatrix.h"
#include "../../math/expressions/Forward.h"
#include "../../math/expressions/MatMatMultExpr.h"
#include "../../math/expressions/MatScalarMultExpr.h"
#include "../../math/functors/DeclDiag.h"
#include "../../math/functors/DeclHerm.h"
#include "../../math/functors/DeclLow.h"
#include "../../math/functors/DeclSym.h"
#include "../../math/functors/DeclUpp.h"
#include "../../math/functors/Noop.h"
#include "../../math/shims/Conjugate.h"
#include "../../math/shims/Reset.h"
#include "../../math/shims/Serial.h"
#include "../../math/SIMD.h"
#include "../../math/traits/MultTrait.h"
#include "../../math/typetraits/HasConstDataAccess.h"
#include "../../math/typetraits/HasMutableDataAccess.h"
#include "../../math/typetraits/HasSIMDAdd.h"
#include "../../math/typetraits/HasSIMDMult.h"
#include "../../math/typetraits/IsAligned.h"
#include "../../math/typetraits/IsBLASCompatible.h"
#include "../../math/typetraits/IsComputation.h"
#include "../../math/typetraits/IsDiagonal.h"
#include "../../math/typetraits/IsExpression.h"
#include "../../math/typetraits/IsLower.h"
#include "../../math/typetraits/IsResizable.h"
#include "../../math/typetraits/IsRowMajorMatrix.h"
#include "../../math/typetraits/IsSIMDCombinable.h"
#include "../../math/typetraits/IsStrictlyLower.h"
#include "../../math/typetraits/IsStrictlyTriangular.h"
#include "../../math/typetraits/IsStrictlyUpper.h"
#include "../../math/typetraits/IsSymmetric.h"
#include "../../math/typetraits/IsTriangular.h"
#include "../../math/typetraits/IsUniLower.h"
#include "../../math/typetraits/IsUniUpper.h"
#include "../../math/typetraits/IsUpper.h"
#include "../../math/typetraits/RequiresEvaluation.h"
#include "../../math/typetraits/Size.h"
#include "../../math/views/Check.h"
#include "../../system/BLAS.h"
#include "../../system/Blocking.h"
#include "../../system/Debugging.h"
#include "../../system/Optimizations.h"
#include "../../system/Thresholds.h"
#include "../../util/algorithms/Max.h"
#include "../../util/algorithms/Min.h"
#include "../../util/Assert.h"
#include "../../util/Complex.h"
#include "../../util/constraints/Numeric.h"
#include "../../util/constraints/SameType.h"
#include "../../util/DisableIf.h"
#include "../../util/EnableIf.h"
#include "../../util/FunctionTrace.h"
#include "../../util/mpl/And.h"
#include "../../util/mpl/Bool.h"
#include "../../util/mpl/If.h"
#include "../../util/mpl/Not.h"
#include "../../util/mpl/Or.h"
#include "../../util/TrueType.h"
#include "../../util/Types.h"
#include "../../util/typetraits/IsBuiltin.h"
#include "../../util/typetraits/IsComplex.h"
#include "../../util/typetraits/IsComplexDouble.h"
#include "../../util/typetraits/IsComplexFloat.h"
#include "../../util/typetraits/IsDouble.h"
#include "../../util/typetraits/IsFloat.h"
#include "../../util/typetraits/IsIntegral.h"
#include "../../util/typetraits/IsSame.h"


namespace blaze {

//=================================================================================================
//
//  CLASS TDMATTDMATMULTEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for transpose dense matrix-transpose dense matrix multiplications.
// \ingroup dense_matrix_expression
//
// The TDMatTDMatMultExpr class represents the compile time expression for multiplications between
// two column-major dense matrices.
*/
template< typename MT1  // Type of the left-hand side dense matrix
        , typename MT2  // Type of the right-hand side dense matrix
        , bool SF       // Symmetry flag
        , bool HF       // Hermitian flag
        , bool LF       // Lower flag
        , bool UF >     // Upper flag
class TDMatTDMatMultExpr
   : public MatMatMultExpr< DenseMatrix< TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF>, true > >
   , private Computation
{
 private:
   //**Type definitions****************************************************************************
   using RT1 = ResultType_<MT1>;     //!< Result type of the left-hand side dense matrix expression.
   using RT2 = ResultType_<MT2>;     //!< Result type of the right-hand side dense matrix expression.
   using ET1 = ElementType_<RT1>;    //!< Element type of the left-hand side dense matrix expression.
   using ET2 = ElementType_<RT2>;    //!< Element type of the right-hand side dense matrix expression.
   using CT1 = CompositeType_<MT1>;  //!< Composite type of the left-hand side dense matrix expression.
   using CT2 = CompositeType_<MT2>;  //!< Composite type of the right-hand side dense matrix expression.
   //**********************************************************************************************

   //**********************************************************************************************
   //! Compilation switch for the composite type of the left-hand side dense matrix expression.
   enum : bool { evaluateLeft = IsComputation<MT1>::value || RequiresEvaluation<MT1>::value };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Compilation switch for the composite type of the right-hand side dense matrix expression.
   enum : bool { evaluateRight = IsComputation<MT2>::value || RequiresEvaluation<MT2>::value };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Compilation switches for the kernel generation.
   enum : bool {
      SYM  = ( SF && !( HF || LF || UF )    ),  //!< Flag for symmetric matrices.
      HERM = ( HF && !( LF || UF )          ),  //!< Flag for Hermitian matrices.
      LOW  = ( LF || ( ( SF || HF ) && UF ) ),  //!< Flag for lower matrices.
      UPP  = ( UF || ( ( SF || HF ) && LF ) )   //!< Flag for upper matrices.
   };
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! The CanExploitSymmetry struct is a helper struct for the selection of the optimal
       evaluation strategy. In case the target matrix is row-major and either of the two
       matrix operands is symmetric, \a value is set to 1 and an optimized evaluation
       strategy is selected. Otherwise \a value is set to 0 and the default strategy is
       chosen. */
   template< typename T1, typename T2, typename T3 >
   struct CanExploitSymmetry {
      enum : bool { value = IsRowMajorMatrix<T1>::value &&
                            ( IsSymmetric<T2>::value || IsSymmetric<T3>::value ) };
   };
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! The IsEvaluationRequired struct is a helper struct for the selection of the parallel
       evaluation strategy. In case either of the two matrix operands requires an intermediate
       evaluation, the nested \value will be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3 >
   struct IsEvaluationRequired {
      enum : bool { value = ( evaluateLeft || evaluateRight ) &&
                            CanExploitSymmetry<T1,T2,T3>::value };
   };
   /*! \endcond */
   //**********************************************************************************************

      //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! In case the types of all three involved matrices are suited for a BLAS kernel, the nested
       \a value will be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3 >
   struct UseBlasKernel {
      enum : bool { value = BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION &&
                            !SYM && !HERM && !LOW && !UPP &&
                            IsContiguous<T1>::value && HasMutableDataAccess<T1>::value &&
                            IsContiguous<T2>::value && HasConstDataAccess<T2>::value &&
                            IsContiguous<T3>::value && HasConstDataAccess<T3>::value &&
                            !IsDiagonal<T2>::value && !IsDiagonal<T3>::value &&
                            T1::simdEnabled && T2::simdEnabled && T3::simdEnabled &&
                            IsBLASCompatible< ElementType_<T1> >::value &&
                            IsBLASCompatible< ElementType_<T2> >::value &&
                            IsBLASCompatible< ElementType_<T3> >::value &&
                            IsSame< ElementType_<T1>, ElementType_<T2> >::value &&
                            IsSame< ElementType_<T1>, ElementType_<T3> >::value };
   };
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! In case all three involved data types are suited for a vectorized computation of the
       matrix multiplication, the nested \value will be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3 >
   struct UseVectorizedDefaultKernel {
      enum : bool { value = useOptimizedKernels &&
                            !IsDiagonal<T2>::value && !IsDiagonal<T3>::value &&
                            T1::simdEnabled && T2::simdEnabled && T3::simdEnabled &&
                            IsSIMDCombinable< ElementType_<T1>
                                            , ElementType_<T2>
                                            , ElementType_<T3> >::value &&
                            HasSIMDAdd< ElementType_<T2>, ElementType_<T3> >::value &&
                            HasSIMDMult< ElementType_<T2>, ElementType_<T3> >::value };
   };
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Type of the functor for forwarding an expression to another assign kernel.
   /*! In case a temporary matrix needs to be created, this functor is used to forward the
       resulting expression to another assign kernel. */
   using ForwardFunctor = IfTrue_< HERM
                                 , DeclHerm
                                 , IfTrue_< SYM
                                          , DeclSym
                                          , IfTrue_< LOW
                                                   , IfTrue_< UPP
                                                            , DeclDiag
                                                            , DeclLow >
                                                   , IfTrue_< UPP
                                                            , DeclUpp
                                                            , Noop > > > >;
   /*! \endcond */
   //**********************************************************************************************

 public:
   //**Type definitions****************************************************************************
   //! Type of this TDMatTDMatMultExpr instance.
   using This = TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF>;

   using ResultType    = MultTrait_<RT1,RT2>;         //!< Result type for expression template evaluations.
   using OppositeType  = OppositeType_<ResultType>;   //!< Result type with opposite storage order for expression template evaluations.
   using TransposeType = TransposeType_<ResultType>;  //!< Transpose type for expression template evaluations.
   using ElementType   = ElementType_<ResultType>;    //!< Resulting element type.
   using SIMDType      = SIMDTrait_<ElementType>;     //!< Resulting SIMD element type.
   using ReturnType    = const ElementType;           //!< Return type for expression template evaluations.
   using CompositeType = const ResultType;            //!< Data type for composite expression templates.

   //! Composite type of the left-hand side dense matrix expression.
   using LeftOperand = If_< IsExpression<MT1>, const MT1, const MT1& >;

   //! Composite type of the right-hand side dense matrix expression.
   using RightOperand = If_< IsExpression<MT2>, const MT2, const MT2& >;

   //! Type for the assignment of the left-hand side dense matrix operand.
   using LT = IfTrue_< evaluateLeft, const RT1, CT1 >;

   //! Type for the assignment of the right-hand side dense matrix operand.
   using RT = IfTrue_< evaluateRight, const RT2, CT2 >;
   //**********************************************************************************************

   //**Compilation flags***************************************************************************
   //! Compilation switch for the expression template evaluation strategy.
   enum : bool { simdEnabled = !IsDiagonal<MT1>::value &&
                               MT1::simdEnabled && MT2::simdEnabled &&
                               HasSIMDAdd<ET1,ET2>::value &&
                               HasSIMDMult<ET1,ET2>::value };

   //! Compilation switch for the expression template assignment strategy.
   enum : bool { smpAssignable = !evaluateLeft  && MT1::smpAssignable &&
                                 !evaluateRight && MT2::smpAssignable };
   //**********************************************************************************************

   //**SIMD properties*****************************************************************************
   //! The number of elements packed within a single SIMD element.
   enum : size_t { SIMDSIZE = SIMDTrait<ElementType>::size };
   //**********************************************************************************************

   //**Constructor*********************************************************************************
   /*!\brief Constructor for the TDMatTDMatMultExpr class.
   //
   // \param lhs The left-hand side operand of the multiplication expression.
   // \param rhs The right-hand side operand of the multiplication expression.
   */
   explicit inline TDMatTDMatMultExpr( const MT1& lhs, const MT2& rhs ) noexcept
      : lhs_( lhs )  // Left-hand side dense matrix of the multiplication expression
      , rhs_( rhs )  // Right-hand side dense matrix of the multiplication expression
   {
      BLAZE_INTERNAL_ASSERT( lhs.columns() == rhs.rows(), "Invalid matrix sizes" );
   }
   //**********************************************************************************************

   //**Access operator*****************************************************************************
   /*!\brief 2D-access to the matrix elements.
   //
   // \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
   // \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
   // \return The resulting value.
   */
   inline ReturnType operator()( size_t i, size_t j ) const {
      BLAZE_INTERNAL_ASSERT( i < lhs_.rows()   , "Invalid row access index"    );
      BLAZE_INTERNAL_ASSERT( j < rhs_.columns(), "Invalid column access index" );

      if( IsDiagonal<MT1>::value ) {
         return lhs_(i,i) * rhs_(i,j);
      }
      else if( IsDiagonal<MT2>::value ) {
         return lhs_(i,j) * rhs_(j,j);
      }
      else if( IsTriangular<MT1>::value || IsTriangular<MT2>::value ) {
         const size_t begin( ( IsUpper<MT1>::value )
                             ?( ( IsLower<MT2>::value )
                                ?( max( ( IsStrictlyUpper<MT1>::value ? i+1UL : i )
                                      , ( IsStrictlyLower<MT2>::value ? j+1UL : j ) ) )
                                :( IsStrictlyUpper<MT1>::value ? i+1UL : i ) )
                             :( ( IsLower<MT2>::value )
                                ?( IsStrictlyLower<MT2>::value ? j+1UL : j )
                                :( 0UL ) ) );
         const size_t end( ( IsLower<MT1>::value )
                           ?( ( IsUpper<MT2>::value )
                              ?( min( ( IsStrictlyLower<MT1>::value ? i : i+1UL )
                                    , ( IsStrictlyUpper<MT2>::value ? j : j+1UL ) ) )
                              :( IsStrictlyLower<MT1>::value ? i : i+1UL ) )
                           :( ( IsUpper<MT2>::value )
                              ?( IsStrictlyUpper<MT2>::value ? j : j+1UL )
                              :( lhs_.columns() ) ) );

         if( begin >= end ) return ElementType();

         const size_t n( end - begin );

         return subvector( row( lhs_, i, unchecked ), begin, n, unchecked ) *
                subvector( column( rhs_, j, unchecked ), begin, n, unchecked );
      }
      else {
         return row( lhs_, i, unchecked ) * column( rhs_, j, unchecked );
      }
   }
   //**********************************************************************************************

   //**At function*********************************************************************************
   /*!\brief Checked access to the matrix elements.
   //
   // \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
   // \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
   // \return The resulting value.
   // \exception std::out_of_range Invalid matrix access index.
   */
   inline ReturnType at( size_t i, size_t j ) const {
      if( i >= lhs_.rows() ) {
         BLAZE_THROW_OUT_OF_RANGE( "Invalid row access index" );
      }
      if( j >= rhs_.columns() ) {
         BLAZE_THROW_OUT_OF_RANGE( "Invalid column access index" );
      }
      return (*this)(i,j);
   }
   //**********************************************************************************************

   //**Rows function*******************************************************************************
   /*!\brief Returns the current number of rows of the matrix.
   //
   // \return The number of rows of the matrix.
   */
   inline size_t rows() const noexcept {
      return lhs_.rows();
   }
   //**********************************************************************************************

   //**Columns function****************************************************************************
   /*!\brief Returns the current number of columns of the matrix.
   //
   // \return The number of columns of the matrix.
   */
   inline size_t columns() const noexcept {
      return rhs_.columns();
   }
   //**********************************************************************************************

   //**Left operand access*************************************************************************
   /*!\brief Returns the left-hand side transpose dense matrix operand.
   //
   // \return The left-hand side transpose dense matrix operand.
   */
   inline LeftOperand leftOperand() const noexcept {
      return lhs_;
   }
   //**********************************************************************************************

   //**Right operand access************************************************************************
   /*!\brief Returns the right-hand side transpose dense matrix operand.
   //
   // \return The right-hand side transpose dense matrix operand.
   */
   inline RightOperand rightOperand() const noexcept {
      return rhs_;
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression can alias with the given address \a alias.
   //
   // \param alias The alias to be checked.
   // \return \a true in case the expression can alias, \a false otherwise.
   */
   template< typename T >
   inline bool canAlias( const T* alias ) const noexcept {
      return ( lhs_.isAliased( alias ) || rhs_.isAliased( alias ) );
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression is aliased with the given address \a alias.
   //
   // \param alias The alias to be checked.
   // \return \a true in case an alias effect is detected, \a false otherwise.
   */
   template< typename T >
   inline bool isAliased( const T* alias ) const noexcept {
      return ( lhs_.isAliased( alias ) || rhs_.isAliased( alias ) );
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the operands of the expression are properly aligned in memory.
   //
   // \return \a true in case the operands are aligned, \a false if not.
   */
   inline bool isAligned() const noexcept {
      return lhs_.isAligned() && rhs_.isAligned();
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression can be used in SMP assignments.
   //
   // \return \a true in case the expression can be used in SMP assignments, \a false if not.
   */
   inline bool canSMPAssign() const noexcept {
      return ( !BLAZE_BLAS_MODE ||
               !BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION ||
               !BLAZE_BLAS_IS_PARALLEL ||
               ( rows() * columns() < TDMATTDMATMULT_THRESHOLD ) ) &&
             ( rows() * columns() >= SMP_TDMATTDMATMULT_THRESHOLD ) &&
             !IsDiagonal<MT1>::value && !IsDiagonal<MT2>::value;
   }
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   LeftOperand  lhs_;  //!< Left-hand side dense matrix of the multiplication expression.
   RightOperand rhs_;  //!< Right-hand side dense matrix of the multiplication expression.
   //**********************************************************************************************

   //**Assignment to dense matrices****************************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Assignment of a transpose dense matrix-transpose dense matrix multiplication to a
   //        dense matrix (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a transpose dense matrix-
   // transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      assign( DenseMatrix<MT,SO>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL ) {
         return;
      }
      else if( rhs.lhs_.columns() == 0UL ) {
         reset( ~lhs );
         return;
      }

      LT A( serial( rhs.lhs_ ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( rhs.rhs_ ) );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      TDMatTDMatMultExpr::selectAssignKernel( ~lhs, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Assignment to dense matrices (kernel selection)*********************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Selection of the kernel for an assignment of a transpose dense matrix-transpose
   //        dense matrix multiplication to a dense matrix (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline void selectAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      if( ( IsDiagonal<MT4>::value ) ||
          ( !BLAZE_DEBUG_MODE && A.rows() <= SIMDSIZE*10UL ) ||
          ( C.rows() * C.columns() < TDMATTDMATMULT_THRESHOLD ) )
         selectSmallAssignKernel( C, A, B );
      else
         selectBlasAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default assignment to dense matrices (general/general)**************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a general transpose dense matrix-general transpose dense matrix
   //        multiplication (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default assignment of a general transpose dense matrix-general
   // transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, Not< IsDiagonal<MT5> > > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( SYM || HERM || LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t kbegin( ( IsLower<MT5>::value )
                              ?( IsStrictlyLower<MT5>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t kend( ( IsUpper<MT5>::value )
                            ?( IsStrictlyUpper<MT5>::value ? j : j+1UL )
                            :( K ) );
         BLAZE_INTERNAL_ASSERT( kbegin <= kend, "Invalid loop indices detected" );

         if( IsStrictlyTriangular<MT5>::value && kbegin == kend ) {
            for( size_t i=0UL; i<M; ++i ) {
               reset( C(i,j) );
            }
            continue;
         }

         {
            const size_t ibegin( ( IsLower<MT4>::value )
                                 ?( ( IsStrictlyLower<MT4>::value )
                                    ?( LOW ? max(j,kbegin+1UL) : kbegin+1UL )
                                    :( LOW ? max(j,kbegin) : kbegin ) )
                                 :( LOW ? j : 0UL ) );
            const size_t iend( ( IsUpper<MT4>::value )
                               ?( ( IsStrictlyUpper<MT4>::value )
                                  ?( UPP ? min(j+1UL,kbegin) : kbegin )
                                  :( UPP ? min(j,kbegin)+1UL : kbegin+1UL ) )
                               :( UPP ? j+1UL : M ) );

            if( ( IsLower<MT4>::value && IsLower<MT5>::value ) || LOW ) {
               for( size_t i=0UL; i<ibegin; ++i ) {
                  reset( C(i,j) );
               }
            }
            else if( IsStrictlyLower<MT4>::value ) {
               reset( C(0UL,j) );
            }
            for( size_t i=ibegin; i<iend; ++i ) {
               C(i,j) = A(i,kbegin) * B(kbegin,j);
            }
            if( ( IsUpper<MT4>::value && IsUpper<MT5>::value ) || UPP ) {
               for( size_t i=iend; i<M; ++i ) {
                  reset( C(i,j) );
               }
            }
            else if( IsStrictlyUpper<MT4>::value ) {
               reset( C(M-1UL,j) );
            }
         }

         for( size_t k=kbegin+1UL; k<kend; ++k )
         {
            const size_t ibegin( ( IsLower<MT4>::value )
                                 ?( ( IsStrictlyLower<MT4>::value )
                                    ?( SYM || HERM || LOW ? max( j, k+1UL ) : k+1UL )
                                    :( SYM || HERM || LOW ? max( j, k ) : k ) )
                                 :( SYM || HERM || LOW ? j : 0UL ) );
            const size_t iend( ( IsUpper<MT4>::value )
                               ?( ( IsStrictlyUpper<MT4>::value )
                                  ?( UPP ? min(j+1UL,k-1UL) : k-1UL )
                                  :( UPP ? min(j+1UL,k) : k ) )
                               :( UPP ? j+1UL : M ) );

            if( ( SYM || HERM || LOW || UPP ) && ( ibegin > iend ) ) continue;
            BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

            for( size_t i=ibegin; i<iend; ++i ) {
               C(i,j) += A(i,k) * B(k,j);
            }
            if( IsUpper<MT4>::value ) {
               C(iend,j) = A(iend,k) * B(k,j);
            }
         }
      }

      if( SYM || HERM ) {
         for( size_t j=1UL; j<N; ++j ) {
            for( size_t i=0UL; i<j; ++i ) {
               C(i,j) = HERM ? conj( C(j,i) ) : C(j,i);
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default assignment to dense matrices (general/diagonal)*************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a general transpose dense matrix-diagonal transpose dense matrix
   //        multiplication (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default assignment of a general transpose dense matrix-diagonal
   // transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, IsDiagonal<MT5> > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t ibegin( ( IsLower<MT4>::value )
                              ?( IsStrictlyLower<MT4>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t iend( ( IsUpper<MT4>::value )
                            ?( IsStrictlyUpper<MT4>::value ? j : j+1UL )
                            :( M ) );
         BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

         if( IsLower<MT4>::value ) {
            for( size_t i=0UL; i<ibegin; ++i ) {
               reset( C(i,j) );
            }
         }
         for( size_t i=ibegin; i<iend; ++i ) {
            C(i,j) = A(i,j) * B(j,j);
         }
         if( IsUpper<MT4>::value ) {
            for( size_t i=iend; i<M; ++i ) {
               reset( C(i,j) );
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default assignment to dense matrices (diagonal/general)*************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a diagonal transpose dense matrix-general transpose dense matrix
   //        multiplication (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default assignment of a diagonal transpose dense matrix-general
   // transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< IsDiagonal<MT4>, Not< IsDiagonal<MT5> > > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t ibegin( ( IsLower<MT5>::value )
                              ?( IsStrictlyLower<MT5>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t iend( ( IsUpper<MT5>::value )
                            ?( IsStrictlyUpper<MT5>::value ? j : j+1UL )
                            :( M ) );
         BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

         if( IsLower<MT4>::value ) {
            for( size_t i=0UL; i<ibegin; ++i ) {
               reset( C(i,j) );
            }
         }
         for( size_t i=ibegin; i<iend; ++i ) {
            C(i,j) = A(i,i) * B(i,j);
         }
         if( IsUpper<MT4>::value ) {
            for( size_t i=iend; i<M; ++i ) {
               reset( C(i,j) );
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default assignment to dense matrices (diagonal/diagonal)************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a diagonal transpose dense matrix-diagonal transpose dense
   //        matrix multiplication (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default assignment of a diagonal transpose dense matrix-
   // diagonal transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< IsDiagonal<MT4>, IsDiagonal<MT5> > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      reset( C );

      for( size_t i=0UL; i<A.rows(); ++i ) {
         C(i,i) = A(i,i) * B(i,i);
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default assignment to dense matrices (small matrices)***************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a small transpose dense matrix-transpose dense matrix
   //        multiplication (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the assignment of a transpose dense
   // matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectDefaultAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default assignment to row-major dense matrices (small matrices)******************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default assignment of a small transpose dense matrix-transpose dense
   //        matrix multiplication (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default assignment of a transpose dense matrix-
   // transpose dense matrix multiplication expression to a row-major dense matrix. This kernel
   // is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallAssignKernel( DenseMatrix<MT3,false>& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT4 );
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT5 );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType_<MT4> );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType_<MT5> );

      const ForwardFunctor fwd;

      if( IsResizable<MT4>::value && !IsResizable<MT5>::value ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         assign( ~C, fwd( A * tmp ) );
      }
      else if( !IsResizable<MT4>::value && IsResizable<MT5>::value ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         assign( ~C, fwd( tmp * B ) );
      }
      else if( B.rows() * B.columns() <= A.rows() * A.columns() ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         assign( ~C, fwd( A * tmp ) );
      }
      else {
         const OppositeType_<MT4> tmp( serial( A ) );
         assign( ~C, fwd( tmp * B ) );
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default assignment to column-major dense matrices (small matrices)***************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default assignment of a small transpose dense matrix-transpose dense
   //        matrix multiplication (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default assignment of a transpose dense matrix-
   // transpose dense matrix multiplication expression to a column-major dense matrix. This
   // kernel is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallAssignKernel( DenseMatrix<MT3,true>& C, const MT4& A, const MT5& B )
   {
      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT4>::value );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( SYM || HERM || LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      const size_t ipos( remainder ? ( M & size_t(-SIMDSIZE) ) : M );
      BLAZE_INTERNAL_ASSERT( !remainder || ( M - ( M % SIMDSIZE ) ) == ipos, "Invalid end calculation" );

      if( LOW && UPP && M > SIMDSIZE*3UL ) {
         reset( ~C );
      }

      {
         size_t i( 0UL );

         if( IsIntegral<ElementType>::value )
         {
            for( ; !SYM && !HERM && !LOW && !UPP && (i+SIMDSIZE*7UL) < ipos; i+=SIMDSIZE*8UL ) {
               for( size_t j=0UL; j<N; ++j )
               {
                  const size_t kbegin( ( IsLower<MT5>::value )
                                       ?( ( IsUpper<MT4>::value )
                                          ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                          :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                       :( IsUpper<MT4>::value ? i : 0UL ) );
                  const size_t kend( ( IsUpper<MT5>::value )
                                     ?( ( IsLower<MT4>::value )
                                        ?( min( i+SIMDSIZE*8UL, K, ( IsStrictlyUpper<MT5>::value ? j : j+1UL ) ) )
                                        :( IsStrictlyUpper<MT5>::value ? j : j+1UL ) )
                                     :( IsLower<MT4>::value ? min( i+SIMDSIZE*8UL, K ) : K ) );

                  SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

                  for( size_t k=kbegin; k<kend; ++k ) {
                     const SIMDType b1( set( B(k,j) ) );
                     xmm1 += A.load(i             ,k) * b1;
                     xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
                     xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
                     xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
                     xmm5 += A.load(i+SIMDSIZE*4UL,k) * b1;
                     xmm6 += A.load(i+SIMDSIZE*5UL,k) * b1;
                     xmm7 += A.load(i+SIMDSIZE*6UL,k) * b1;
                     xmm8 += A.load(i+SIMDSIZE*7UL,k) * b1;
                  }

                  (~C).store( i             , j, xmm1 );
                  (~C).store( i+SIMDSIZE    , j, xmm2 );
                  (~C).store( i+SIMDSIZE*2UL, j, xmm3 );
                  (~C).store( i+SIMDSIZE*3UL, j, xmm4 );
                  (~C).store( i+SIMDSIZE*4UL, j, xmm5 );
                  (~C).store( i+SIMDSIZE*5UL, j, xmm6 );
                  (~C).store( i+SIMDSIZE*6UL, j, xmm7 );
                  (~C).store( i+SIMDSIZE*7UL, j, xmm8 );
               }
            }
         }

         for( ; !SYM && !HERM && !LOW && !UPP && (i+SIMDSIZE*4UL) < ipos; i+=SIMDSIZE*5UL )
         {
            size_t j( 0UL );

            for( ; (j+2UL) <= N; j+=2UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*5UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*5UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( A.load(i             ,k) );
                  const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
                  const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
                  const SIMDType a4( A.load(i+SIMDSIZE*3UL,k) );
                  const SIMDType a5( A.load(i+SIMDSIZE*4UL,k) );
                  const SIMDType b1( set( B(k,j    ) ) );
                  const SIMDType b2( set( B(k,j+1UL) ) );
                  xmm1  += a1 * b1;
                  xmm2  += a2 * b1;
                  xmm3  += a3 * b1;
                  xmm4  += a4 * b1;
                  xmm5  += a5 * b1;
                  xmm6  += a1 * b2;
                  xmm7  += a2 * b2;
                  xmm8  += a3 * b2;
                  xmm9  += a4 * b2;
                  xmm10 += a5 * b2;
               }

               (~C).store( i             , j    , xmm1  );
               (~C).store( i+SIMDSIZE    , j    , xmm2  );
               (~C).store( i+SIMDSIZE*2UL, j    , xmm3  );
               (~C).store( i+SIMDSIZE*3UL, j    , xmm4  );
               (~C).store( i+SIMDSIZE*4UL, j    , xmm5  );
               (~C).store( i             , j+1UL, xmm6  );
               (~C).store( i+SIMDSIZE    , j+1UL, xmm7  );
               (~C).store( i+SIMDSIZE*2UL, j+1UL, xmm8  );
               (~C).store( i+SIMDSIZE*3UL, j+1UL, xmm9  );
               (~C).store( i+SIMDSIZE*4UL, j+1UL, xmm10 );
            }

            if( j < N )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*5UL, K ) ):( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType b1( set( B(k,j) ) );
                  xmm1 += A.load(i             ,k) * b1;
                  xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
                  xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
                  xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
                  xmm5 += A.load(i+SIMDSIZE*4UL,k) * b1;
               }

               (~C).store( i             , j, xmm1 );
               (~C).store( i+SIMDSIZE    , j, xmm2 );
               (~C).store( i+SIMDSIZE*2UL, j, xmm3 );
               (~C).store( i+SIMDSIZE*3UL, j, xmm4 );
               (~C).store( i+SIMDSIZE*4UL, j, xmm5 );
            }
         }

         for( ; !( LOW && UPP ) && (i+SIMDSIZE*3UL) < ipos; i+=SIMDSIZE*4UL )
         {
            const size_t jend( SYM || HERM || LOW ? min(i+SIMDSIZE*4UL,N) : N );
            size_t j( UPP ? i : 0UL );

            for( ; (j+2UL) <= jend; j+=2UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*4UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*4UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( A.load(i             ,k) );
                  const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
                  const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
                  const SIMDType a4( A.load(i+SIMDSIZE*3UL,k) );
                  const SIMDType b1( set( B(k,j    ) ) );
                  const SIMDType b2( set( B(k,j+1UL) ) );
                  xmm1 += a1 * b1;
                  xmm2 += a2 * b1;
                  xmm3 += a3 * b1;
                  xmm4 += a4 * b1;
                  xmm5 += a1 * b2;
                  xmm6 += a2 * b2;
                  xmm7 += a3 * b2;
                  xmm8 += a4 * b2;
               }

               (~C).store( i             , j    , xmm1 );
               (~C).store( i+SIMDSIZE    , j    , xmm2 );
               (~C).store( i+SIMDSIZE*2UL, j    , xmm3 );
               (~C).store( i+SIMDSIZE*3UL, j    , xmm4 );
               (~C).store( i             , j+1UL, xmm5 );
               (~C).store( i+SIMDSIZE    , j+1UL, xmm6 );
               (~C).store( i+SIMDSIZE*2UL, j+1UL, xmm7 );
               (~C).store( i+SIMDSIZE*3UL, j+1UL, xmm8 );
            }

            if( j < jend )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*4UL, K ) ):( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType b1( set( B(k,j) ) );
                  xmm1 += A.load(i             ,k) * b1;
                  xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
                  xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
                  xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
               }

               (~C).store( i             , j, xmm1 );
               (~C).store( i+SIMDSIZE    , j, xmm2 );
               (~C).store( i+SIMDSIZE*2UL, j, xmm3 );
               (~C).store( i+SIMDSIZE*3UL, j, xmm4 );
            }
         }

         for( ; (i+SIMDSIZE*2UL) < ipos; i+=SIMDSIZE*3UL )
         {
            const size_t jend( SYM || HERM || LOW ? min(i+SIMDSIZE*3UL,N) : N );
            size_t j( UPP ? i : 0UL );

            for( ; (j+2UL) <= jend; j+=2UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*3UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*3UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( A.load(i             ,k) );
                  const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
                  const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
                  const SIMDType b1( set( B(k,j    ) ) );
                  const SIMDType b2( set( B(k,j+1UL) ) );
                  xmm1 += a1 * b1;
                  xmm2 += a2 * b1;
                  xmm3 += a3 * b1;
                  xmm4 += a1 * b2;
                  xmm5 += a2 * b2;
                  xmm6 += a3 * b2;
               }

               (~C).store( i             , j    , xmm1 );
               (~C).store( i+SIMDSIZE    , j    , xmm2 );
               (~C).store( i+SIMDSIZE*2UL, j    , xmm3 );
               (~C).store( i             , j+1UL, xmm4 );
               (~C).store( i+SIMDSIZE    , j+1UL, xmm5 );
               (~C).store( i+SIMDSIZE*2UL, j+1UL, xmm6 );
            }

            if( j < jend )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*3UL, K ) ):( K ) );

               SIMDType xmm1, xmm2, xmm3;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType b1( set( B(k,j) ) );
                  xmm1 += A.load(i             ,k) * b1;
                  xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
                  xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
               }

               (~C).store( i             , j, xmm1 );
               (~C).store( i+SIMDSIZE    , j, xmm2 );
               (~C).store( i+SIMDSIZE*2UL, j, xmm3 );
            }
         }

         for( ; (i+SIMDSIZE) < ipos; i+=SIMDSIZE*2UL )
         {
            const size_t jend( SYM || HERM || LOW ? min(i+SIMDSIZE*2UL,N) : N );
            size_t j( UPP ? i : 0UL );

            for( ; (j+4UL) <= jend; j+=4UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( A.load(i         ,k) );
                  const SIMDType a2( A.load(i+SIMDSIZE,k) );
                  const SIMDType b1( set( B(k,j    ) ) );
                  const SIMDType b2( set( B(k,j+1UL) ) );
                  const SIMDType b3( set( B(k,j+2UL) ) );
                  const SIMDType b4( set( B(k,j+3UL) ) );
                  xmm1 += a1 * b1;
                  xmm2 += a2 * b1;
                  xmm3 += a1 * b2;
                  xmm4 += a2 * b2;
                  xmm5 += a1 * b3;
                  xmm6 += a2 * b3;
                  xmm7 += a1 * b4;
                  xmm8 += a2 * b4;
               }

               (~C).store( i         , j    , xmm1 );
               (~C).store( i+SIMDSIZE, j    , xmm2 );
               (~C).store( i         , j+1UL, xmm3 );
               (~C).store( i+SIMDSIZE, j+1UL, xmm4 );
               (~C).store( i         , j+2UL, xmm5 );
               (~C).store( i+SIMDSIZE, j+2UL, xmm6 );
               (~C).store( i         , j+3UL, xmm7 );
               (~C).store( i+SIMDSIZE, j+3UL, xmm8 );
            }

            for( ; (j+3UL) <= jend; j+=3UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( A.load(i         ,k) );
                  const SIMDType a2( A.load(i+SIMDSIZE,k) );
                  const SIMDType b1( set( B(k,j    ) ) );
                  const SIMDType b2( set( B(k,j+1UL) ) );
                  const SIMDType b3( set( B(k,j+2UL) ) );
                  xmm1 += a1 * b1;
                  xmm2 += a2 * b1;
                  xmm3 += a1 * b2;
                  xmm4 += a2 * b2;
                  xmm5 += a1 * b3;
                  xmm6 += a2 * b3;
               }

               (~C).store( i         , j    , xmm1 );
               (~C).store( i+SIMDSIZE, j    , xmm2 );
               (~C).store( i         , j+1UL, xmm3 );
               (~C).store( i+SIMDSIZE, j+1UL, xmm4 );
               (~C).store( i         , j+2UL, xmm5 );
               (~C).store( i+SIMDSIZE, j+2UL, xmm6 );
            }

            for( ; (j+2UL) <= jend; j+=2UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;
               size_t k( kbegin );

               for( ; (k+2UL) <= kend; k+=2UL ) {
                  const SIMDType a1( A.load(i         ,k    ) );
                  const SIMDType a2( A.load(i+SIMDSIZE,k    ) );
                  const SIMDType a3( A.load(i         ,k+1UL) );
                  const SIMDType a4( A.load(i+SIMDSIZE,k+1UL) );
                  const SIMDType b1( set( B(k    ,j    ) ) );
                  const SIMDType b2( set( B(k    ,j+1UL) ) );
                  const SIMDType b3( set( B(k+1UL,j    ) ) );
                  const SIMDType b4( set( B(k+1UL,j+1UL) ) );
                  xmm1 += a1 * b1;
                  xmm2 += a2 * b1;
                  xmm3 += a1 * b2;
                  xmm4 += a2 * b2;
                  xmm5 += a3 * b3;
                  xmm6 += a4 * b3;
                  xmm7 += a3 * b4;
                  xmm8 += a4 * b4;
               }

               for( ; k<kend; ++k ) {
                  const SIMDType a1( A.load(i         ,k) );
                  const SIMDType a2( A.load(i+SIMDSIZE,k) );
                  const SIMDType b1( set( B(k,j    ) ) );
                  const SIMDType b2( set( B(k,j+1UL) ) );
                  xmm1 += a1 * b1;
                  xmm2 += a2 * b1;
                  xmm3 += a1 * b2;
                  xmm4 += a2 * b2;
               }

               (~C).store( i         , j    , xmm1+xmm5 );
               (~C).store( i+SIMDSIZE, j    , xmm2+xmm6 );
               (~C).store( i         , j+1UL, xmm3+xmm7 );
               (~C).store( i+SIMDSIZE, j+1UL, xmm4+xmm8 );
            }

            if( j < jend )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*2UL, K ) ):( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4;
               size_t k( kbegin );

               for( ; (k+2UL) <= kend; k+=2UL ) {
                  const SIMDType b1( set( B(k    ,j) ) );
                  const SIMDType b2( set( B(k+1UL,j) ) );
                  xmm1 += A.load(i         ,k    ) * b1;
                  xmm2 += A.load(i+SIMDSIZE,k    ) * b1;
                  xmm3 += A.load(i         ,k+1UL) * b2;
                  xmm4 += A.load(i+SIMDSIZE,k+1UL) * b2;
               }

               for( ; k<kend; ++k ) {
                  const SIMDType b1( set( B(k,j) ) );
                  xmm1 += A.load(i         ,k) * b1;
                  xmm2 += A.load(i+SIMDSIZE,k) * b1;
               }

               (~C).store( i         , j, xmm1+xmm3 );
               (~C).store( i+SIMDSIZE, j, xmm2+xmm4 );
            }
         }

         for( ; i<ipos; i+=SIMDSIZE )
         {
            const size_t jend( SYM || HERM || LOW ? min(i+SIMDSIZE,N) : N );
            size_t j( UPP ? i : 0UL );

            for( ; (j+4UL) <= jend; j+=4UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL )
                                  :( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;
               size_t k( kbegin );

               for( ; (k+2UL) <= kend; k+=2UL ) {
                  const SIMDType a1( A.load(i,k    ) );
                  const SIMDType a2( A.load(i,k+1UL) );
                  xmm1 += a1 * set( B(k    ,j    ) );
                  xmm2 += a1 * set( B(k    ,j+1UL) );
                  xmm3 += a1 * set( B(k    ,j+2UL) );
                  xmm4 += a1 * set( B(k    ,j+3UL) );
                  xmm5 += a2 * set( B(k+1UL,j    ) );
                  xmm6 += a2 * set( B(k+1UL,j+1UL) );
                  xmm7 += a2 * set( B(k+1UL,j+2UL) );
                  xmm8 += a2 * set( B(k+1UL,j+3UL) );
               }

               for( ; k<kend; ++k ) {
                  const SIMDType a1( A.load(i,k) );
                  xmm1 += a1 * set( B(k,j    ) );
                  xmm2 += a1 * set( B(k,j+1UL) );
                  xmm3 += a1 * set( B(k,j+2UL) );
                  xmm4 += a1 * set( B(k,j+3UL) );
               }

               (~C).store( i, j    , xmm1+xmm5 );
               (~C).store( i, j+1UL, xmm2+xmm6 );
               (~C).store( i, j+2UL, xmm3+xmm7 );
               (~C).store( i, j+3UL, xmm4+xmm8 );
            }

            for( ; (j+3UL) <= jend; j+=3UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL )
                                  :( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;
               size_t k( kbegin );

               for( ; (k+2UL) <= kend; k+=2UL ) {
                  const SIMDType a1( A.load(i,k    ) );
                  const SIMDType a2( A.load(i,k+1UL) );
                  xmm1 += a1 * set( B(k    ,j    ) );
                  xmm2 += a1 * set( B(k    ,j+1UL) );
                  xmm3 += a1 * set( B(k    ,j+2UL) );
                  xmm4 += a2 * set( B(k+1UL,j    ) );
                  xmm5 += a2 * set( B(k+1UL,j+1UL) );
                  xmm6 += a2 * set( B(k+1UL,j+2UL) );
               }

               for( ; k<kend; ++k ) {
                  const SIMDType a1( A.load(i,k) );
                  xmm1 += a1 * set( B(k,j    ) );
                  xmm2 += a1 * set( B(k,j+1UL) );
                  xmm3 += a1 * set( B(k,j+2UL) );
               }

               (~C).store( i, j    , xmm1+xmm4 );
               (~C).store( i, j+1UL, xmm2+xmm5 );
               (~C).store( i, j+2UL, xmm3+xmm6 );
            }

            for( ; (j+2UL) <= jend; j+=2UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL )
                                  :( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4;
               size_t k( kbegin );

               for( ; (k+2UL) <= kend; k+=2UL ) {
                  const SIMDType a1( A.load(i,k    ) );
                  const SIMDType a2( A.load(i,k+1UL) );
                  xmm1 += a1 * set( B(k    ,j    ) );
                  xmm2 += a1 * set( B(k    ,j+1UL) );
                  xmm3 += a2 * set( B(k+1UL,j    ) );
                  xmm4 += a2 * set( B(k+1UL,j+1UL) );
               }

               for( ; k<kend; ++k ) {
                  const SIMDType a1( A.load(i,k) );
                  xmm1 += a1 * set( B(k,j    ) );
                  xmm2 += a1 * set( B(k,j+1UL) );
               }

               (~C).store( i, j    , xmm1+xmm3 );
               (~C).store( i, j+1UL, xmm2+xmm4 );
            }

            if( j < jend )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );

               SIMDType xmm1, xmm2;
               size_t k( kbegin );

               for( ; (k+2UL) <= K; k+=2UL ) {
                  xmm1 += A.load(i,k    ) * set( B(k    ,j) );
                  xmm2 += A.load(i,k+1UL) * set( B(k+1UL,j) );
               }

               for( ; k<K; ++k ) {
                  xmm1 += A.load(i,k) * set( B(k,j) );
               }

               (~C).store( i, j, xmm1+xmm2 );
            }
         }

         for( ; remainder && i<M; ++i )
         {
            size_t j( LOW && UPP ? i : 0UL );

            for( ; (j+2UL) <= N; j+=2UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL )
                                  :( K ) );

               ElementType value1{};
               ElementType value2{};

               for( size_t k=kbegin; k<kend; ++k ) {
                  value1 += A(i,k) * B(k,j    );
                  value2 += A(i,k) * B(k,j+1UL);
               }

               (~C)(i,j    ) = value1;
               (~C)(i,j+1UL) = value2;
            }

            if( j < N )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );

               ElementType value{};

               for( size_t k=kbegin; k<K; ++k ) {
                  value += A(i,k) * B(k,j);
               }

               (~C)(i,j) = value;
            }
         }
      }

      if( ( SYM || HERM ) && ( M > SIMDSIZE*4UL ) ) {
         for( size_t j=SIMDSIZE*4UL; j<N; ++j ) {
            const size_t iend( ( SIMDSIZE*4UL ) * ( j / (SIMDSIZE*4UL) ) );
            for( size_t i=0UL; i<iend; ++i ) {
               (~C)(i,j) = HERM ? conj( (~C)(j,i) ) : (~C)(j,i);
            }
         }
      }
      else if( LOW && !UPP && M > SIMDSIZE*4UL ) {
         for( size_t j=SIMDSIZE*4UL; j<N; ++j ) {
            const size_t iend( ( SIMDSIZE*4UL ) * ( j / (SIMDSIZE*4UL) ) );
            for( size_t i=0UL; i<iend; ++i ) {
               reset( (~C)(i,j) );
            }
         }
      }
      else if( !LOW && UPP && M > SIMDSIZE*4UL ) {
         for( size_t i=SIMDSIZE*4UL; i<M; ++i ) {
            const size_t jend( ( SIMDSIZE*4UL ) * ( i / (SIMDSIZE*4UL) ) );
            for( size_t j=0UL; j<jend; ++j ) {
               reset( (~C)(i,j) );
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default assignment to dense matrices (large matrices)***************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a large transpose dense matrix-transpose dense matrix
   //        multiplication (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the assignment of a transpose dense
   // matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectLargeAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectDefaultAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default assignment to dense matrices (large matrices)****************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default assignment of a large transpose dense matrix-transpose dense
   //        matrix multiplication (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default assignment of a transpose dense matrix-
   // transpose dense matrix multiplication expression to a dense matrix. This kernel is
   // optimized for large matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectLargeAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      if( SYM )
         smmm( C, A, B, ElementType(1) );
      else if( HERM )
         hmmm( C, A, B, ElementType(1) );
      else if( LOW )
         lmmm( C, A, B, ElementType(1), ElementType(0) );
      else if( UPP )
         ummm( C, A, B, ElementType(1), ElementType(0) );
      else
         mmm( C, A, B, ElementType(1), ElementType(0) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**BLAS-based assignment to dense matrices (default)*******************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default assignment of a transpose dense matrix-transpose dense matrix multiplication
   //        (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the assignment of a large transpose
   // dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseBlasKernel<MT3,MT4,MT5> >
      selectBlasAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectLargeAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**BLAS-based assignment to dense matrices*****************************************************
#if BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION
   /*! \cond BLAZE_INTERNAL */
   /*!\brief BLAS-based assignment of a transpose dense matrix-transpose dense matrix
   //        multiplication (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function performs the transpose dense matrix-transpose dense matrix multiplication
   // based on the according BLAS functionality.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseBlasKernel<MT3,MT4,MT5> >
      selectBlasAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      using ET = ElementType_<MT3>;

      if( IsTriangular<MT4>::value ) {
         assign( C, B );
         trmm( C, A, CblasLeft, ( IsLower<MT4>::value )?( CblasLower ):( CblasUpper ), ET(1) );
      }
      else if( IsTriangular<MT5>::value ) {
         assign( C, A );
         trmm( C, B, CblasRight, ( IsLower<MT5>::value )?( CblasLower ):( CblasUpper ), ET(1) );
      }
      else {
         gemm( C, A, B, ET(1), ET(0) );
      }
   }
   /*! \endcond */
#endif
   //**********************************************************************************************

   //**Assignment to sparse matrices***************************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Assignment of a transpose dense matrix-transpose dense matrix multiplication to a
   //        sparse matrix (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side sparse matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a transpose dense matrix-
   // transpose dense matrix multiplication expression to a sparse matrix.
   */
   template< typename MT  // Type of the target sparse matrix
           , bool SO >    // Storage order of the target sparse matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      assign( SparseMatrix<MT,SO>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      using TmpType = IfTrue_< SO, ResultType, OppositeType >;

      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MATRICES_MUST_HAVE_SAME_STORAGE_ORDER( MT, TmpType );
      BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( TmpType );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      const TmpType tmp( serial( rhs ) );
      assign( ~lhs, fwd( tmp ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Restructuring assignment to row-major matrices**********************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Restructuring assignment of a transpose dense matrix-transpose dense matrix
   //        multiplication to a row-major matrix (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the symmetry-based restructuring assignment of a transpose dense
   // matrix-transpose dense matrix multiplication expression to a row-major matrix. Due to the
   // explicit application of the SFINAE principle this function can only be selected by the
   // compiler in case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      assign( Matrix<MT,false>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         assign( ~lhs, fwd( trans( rhs.lhs_ ) * trans( rhs.rhs_ ) ) );
      else if( IsSymmetric<MT1>::value )
         assign( ~lhs, fwd( trans( rhs.lhs_ ) * rhs.rhs_ ) );
      else
         assign( ~lhs, fwd( rhs.lhs_ * trans( rhs.rhs_ ) ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Addition assignment to dense matrices*******************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Addition assignment of a transpose dense matrix-transpose dense matrix multiplication
   //        to a dense matrix (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be added.
   // \return void
   //
   // This function implements the performance optimized addition assignment of a transpose dense
   // matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT   // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      addAssign( DenseMatrix<MT,SO>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || rhs.lhs_.columns() == 0UL ) {
         return;
      }

      LT A( serial( rhs.lhs_ ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( rhs.rhs_ ) );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      TDMatTDMatMultExpr::selectAddAssignKernel( ~lhs, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Addition assignment to dense matrices (kernel selection)************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Selection of the kernel for an addition assignment of a transpose dense matrix-
   //        transpose dense matrix multiplication to a dense matrix (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline void selectAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      if( ( IsDiagonal<MT4>::value ) ||
          ( !BLAZE_DEBUG_MODE && A.rows() <= SIMDSIZE*10UL ) ||
          ( C.rows() * C.columns() < TDMATTDMATMULT_THRESHOLD ) )
         selectSmallAddAssignKernel( C, A, B );
      else
         selectBlasAddAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (general/general)*****************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a general transpose dense matrix-general transpose
   //        dense matrix multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default addition assignment of a general transpose dense matrix-
   // general transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, Not< IsDiagonal<MT5> > > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t kbegin( ( IsLower<MT5>::value )
                              ?( IsStrictlyLower<MT5>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t kend( ( IsUpper<MT5>::value )
                            ?( IsStrictlyUpper<MT5>::value ? j : j+1UL )
                            :( K ) );
         BLAZE_INTERNAL_ASSERT( kbegin <= kend, "Invalid loop indices detected" );

         for( size_t k=kbegin; k<kend; ++k )
         {
            const size_t ibegin( ( IsLower<MT4>::value )
                                 ?( ( IsStrictlyLower<MT4>::value )
                                    ?( LOW ? max(j,k+1UL) : k+1UL )
                                    :( LOW ? max(j,k) : k ) )
                                 :( LOW ? j : 0UL ) );
            const size_t iend( ( IsUpper<MT4>::value )
                               ?( ( IsStrictlyUpper<MT4>::value )
                                  ?( UPP ? min(j+1UL,k) : k )
                                  :( UPP ? min(j,k)+1UL : k+1UL ) )
                               :( UPP ? j+1UL : M ) );

            if( ( LOW || UPP ) && ibegin >= iend ) continue;
            BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

            const size_t inum( iend - ibegin );
            const size_t ipos( ibegin + ( inum & size_t(-2) ) );

            for( size_t i=ibegin; i<ipos; i+=2UL ) {
               C(i    ,j) += A(i    ,k) * B(k,j);
               C(i+1UL,j) += A(i+1UL,k) * B(k,j);
            }
            if( ipos < iend ) {
               C(ipos,j) += A(ipos,k) * B(k,j);
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (general/diagonal)****************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a general transpose dense matrix-diagonal transpose
   //        dense matrix multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default addition assignment of a general transpose dense matrix-
   // diagonal transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, IsDiagonal<MT5> > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t ibegin( ( IsLower<MT4>::value )
                              ?( IsStrictlyLower<MT4>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t iend( ( IsUpper<MT4>::value )
                            ?( IsStrictlyUpper<MT4>::value ? j : j+1UL )
                            :( M ) );
         BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

         const size_t inum( iend - ibegin );
         const size_t ipos( ibegin + ( inum & size_t(-2) ) );

         for( size_t i=ibegin; i<ipos; i+=2UL ) {
            C(i    ,j) += A(i    ,j) * B(j,j);
            C(i+1UL,j) += A(i+1UL,j) * B(j,j);
         }
         if( ipos < iend ) {
            C(ipos,j) += A(ipos,j) * B(j,j);
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (diagonal/general)****************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a diagonal transpose dense matrix-general transpose
   //        dense matrix multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default addition assignment of a diagonal transpose dense
   // matrix-general transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< IsDiagonal<MT4>, Not< IsDiagonal<MT5> > > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t ibegin( ( IsLower<MT5>::value )
                              ?( IsStrictlyLower<MT5>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t iend( ( IsUpper<MT5>::value )
                            ?( IsStrictlyUpper<MT5>::value ? j : j+1UL )
                            :( M ) );
         BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

         const size_t inum( iend - ibegin );
         const size_t ipos( ibegin + ( inum & size_t(-2) ) );

         for( size_t i=ibegin; i<ipos; i+=2UL ) {
            C(i    ,j) += A(i    ,i    ) * B(i    ,j);
            C(i+1UL,j) += A(i+1UL,i+1UL) * B(i+1UL,j);
         }
         if( ipos < iend ) {
            C(ipos,j) += A(ipos,ipos) * B(ipos,j);
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (diagonal/diagonal)***************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a diagonal transpose dense matrix-diagonal transpose
   //        dense matrix multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default addition assignment of a diagonal transpose dense
   // matrix-diagonal transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< IsDiagonal<MT4>, IsDiagonal<MT5> > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      for( size_t i=0UL; i<A.rows(); ++i ) {
         C(i,i) += A(i,i) * B(i,i);
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (small matrices)******************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a small transpose dense matrix-transpose dense matrix
   //        multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the addition assignment of a transpose
   // dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectDefaultAddAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default addition assignment to row-major dense matrices (small matrices)*********
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default addition assignment of a small transpose dense matrix-transpose
   //        dense matrix multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default addition assignment of a transpose dense
   // matrix-transpose dense matrix multiplication expression to a row-major dense matrix. This
   // kernel is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallAddAssignKernel( DenseMatrix<MT3,false>& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT4 );
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT5 );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType_<MT4> );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType_<MT5> );

      const ForwardFunctor fwd;

      if( IsResizable<MT4>::value && !IsResizable<MT5>::value ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         addAssign( ~C, fwd( A * tmp ) );
      }
      else if( !IsResizable<MT4>::value && IsResizable<MT5>::value ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         addAssign( ~C, fwd( tmp * B ) );
      }
      else if( B.rows() * B.columns() <= A.rows() * A.columns() ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         addAssign( ~C, fwd( A * tmp ) );
      }
      else {
         const OppositeType_<MT4> tmp( serial( A ) );
         addAssign( ~C, fwd( tmp * B ) );
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default addition assignment to column-major dense matrices (small matrices)******
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default addition assignment of a small transpose dense matrix-transpose
   //        dense matrix multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default addition assignment of a transpose dense
   // matrix-transpose dense matrix multiplication expression to a column-major dense matrix.
   // This kernel is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallAddAssignKernel( DenseMatrix<MT3,true>& C, const MT4& A, const MT5& B )
   {
      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT4>::value );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      const size_t ipos( remainder ? ( M & size_t(-SIMDSIZE) ) : M );
      BLAZE_INTERNAL_ASSERT( !remainder || ( M - ( M % SIMDSIZE ) ) == ipos, "Invalid end calculation" );

      size_t i( 0UL );

      if( IsIntegral<ElementType>::value )
      {
         for( ; !LOW && !UPP && (i+SIMDSIZE*7UL) < ipos; i+=SIMDSIZE*8UL ) {
            for( size_t j=0UL; j<N; ++j )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*8UL, K, ( IsStrictlyUpper<MT5>::value ? j : j+1UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j : j+1UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*8UL, K ) : K ) );

               SIMDType xmm1( (~C).load(i             ,j) );
               SIMDType xmm2( (~C).load(i+SIMDSIZE    ,j) );
               SIMDType xmm3( (~C).load(i+SIMDSIZE*2UL,j) );
               SIMDType xmm4( (~C).load(i+SIMDSIZE*3UL,j) );
               SIMDType xmm5( (~C).load(i+SIMDSIZE*4UL,j) );
               SIMDType xmm6( (~C).load(i+SIMDSIZE*5UL,j) );
               SIMDType xmm7( (~C).load(i+SIMDSIZE*6UL,j) );
               SIMDType xmm8( (~C).load(i+SIMDSIZE*7UL,j) );

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType b1( set( B(k,j) ) );
                  xmm1 += A.load(i             ,k) * b1;
                  xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
                  xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
                  xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
                  xmm5 += A.load(i+SIMDSIZE*4UL,k) * b1;
                  xmm6 += A.load(i+SIMDSIZE*5UL,k) * b1;
                  xmm7 += A.load(i+SIMDSIZE*6UL,k) * b1;
                  xmm8 += A.load(i+SIMDSIZE*7UL,k) * b1;
               }

               (~C).store( i             , j, xmm1 );
               (~C).store( i+SIMDSIZE    , j, xmm2 );
               (~C).store( i+SIMDSIZE*2UL, j, xmm3 );
               (~C).store( i+SIMDSIZE*3UL, j, xmm4 );
               (~C).store( i+SIMDSIZE*4UL, j, xmm5 );
               (~C).store( i+SIMDSIZE*5UL, j, xmm6 );
               (~C).store( i+SIMDSIZE*6UL, j, xmm7 );
               (~C).store( i+SIMDSIZE*7UL, j, xmm8 );
            }
         }
      }

      for( ; !LOW && !UPP && (i+SIMDSIZE*4UL) < ipos; i+=SIMDSIZE*5UL )
      {
         size_t j( 0UL );

         for( ; (j+2UL) <= N; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*5UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*5UL, K ) : K ) );

            SIMDType xmm1 ( (~C).load(i             ,j    ) );
            SIMDType xmm2 ( (~C).load(i+SIMDSIZE    ,j    ) );
            SIMDType xmm3 ( (~C).load(i+SIMDSIZE*2UL,j    ) );
            SIMDType xmm4 ( (~C).load(i+SIMDSIZE*3UL,j    ) );
            SIMDType xmm5 ( (~C).load(i+SIMDSIZE*4UL,j    ) );
            SIMDType xmm6 ( (~C).load(i             ,j+1UL) );
            SIMDType xmm7 ( (~C).load(i+SIMDSIZE    ,j+1UL) );
            SIMDType xmm8 ( (~C).load(i+SIMDSIZE*2UL,j+1UL) );
            SIMDType xmm9 ( (~C).load(i+SIMDSIZE*3UL,j+1UL) );
            SIMDType xmm10( (~C).load(i+SIMDSIZE*4UL,j+1UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i             ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
               const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
               const SIMDType a4( A.load(i+SIMDSIZE*3UL,k) );
               const SIMDType a5( A.load(i+SIMDSIZE*4UL,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1  += a1 * b1;
               xmm2  += a2 * b1;
               xmm3  += a3 * b1;
               xmm4  += a4 * b1;
               xmm5  += a5 * b1;
               xmm6  += a1 * b2;
               xmm7  += a2 * b2;
               xmm8  += a3 * b2;
               xmm9  += a4 * b2;
               xmm10 += a5 * b2;
            }

            (~C).store( i             , j    , xmm1  );
            (~C).store( i+SIMDSIZE    , j    , xmm2  );
            (~C).store( i+SIMDSIZE*2UL, j    , xmm3  );
            (~C).store( i+SIMDSIZE*3UL, j    , xmm4  );
            (~C).store( i+SIMDSIZE*4UL, j    , xmm5  );
            (~C).store( i             , j+1UL, xmm6  );
            (~C).store( i+SIMDSIZE    , j+1UL, xmm7  );
            (~C).store( i+SIMDSIZE*2UL, j+1UL, xmm8  );
            (~C).store( i+SIMDSIZE*3UL, j+1UL, xmm9  );
            (~C).store( i+SIMDSIZE*4UL, j+1UL, xmm10 );
         }

         if( j < N )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*5UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i             ,j) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE    ,j) );
            SIMDType xmm3( (~C).load(i+SIMDSIZE*2UL,j) );
            SIMDType xmm4( (~C).load(i+SIMDSIZE*3UL,j) );
            SIMDType xmm5( (~C).load(i+SIMDSIZE*4UL,j) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 += A.load(i             ,k) * b1;
               xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
               xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
               xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
               xmm5 += A.load(i+SIMDSIZE*4UL,k) * b1;
            }

            (~C).store( i             , j, xmm1 );
            (~C).store( i+SIMDSIZE    , j, xmm2 );
            (~C).store( i+SIMDSIZE*2UL, j, xmm3 );
            (~C).store( i+SIMDSIZE*3UL, j, xmm4 );
            (~C).store( i+SIMDSIZE*4UL, j, xmm5 );
         }
      }

      for( ; !LOW && !UPP && (i+SIMDSIZE*3UL) < ipos; i+=SIMDSIZE*4UL )
      {
         size_t j( 0UL );

         for( ; (j+2UL) <= N; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*4UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*4UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i             ,j    ) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE    ,j    ) );
            SIMDType xmm3( (~C).load(i+SIMDSIZE*2UL,j    ) );
            SIMDType xmm4( (~C).load(i+SIMDSIZE*3UL,j    ) );
            SIMDType xmm5( (~C).load(i             ,j+1UL) );
            SIMDType xmm6( (~C).load(i+SIMDSIZE    ,j+1UL) );
            SIMDType xmm7( (~C).load(i+SIMDSIZE*2UL,j+1UL) );
            SIMDType xmm8( (~C).load(i+SIMDSIZE*3UL,j+1UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i             ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
               const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
               const SIMDType a4( A.load(i+SIMDSIZE*3UL,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a3 * b1;
               xmm4 += a4 * b1;
               xmm5 += a1 * b2;
               xmm6 += a2 * b2;
               xmm7 += a3 * b2;
               xmm8 += a4 * b2;
            }

            (~C).store( i             , j    , xmm1 );
            (~C).store( i+SIMDSIZE    , j    , xmm2 );
            (~C).store( i+SIMDSIZE*2UL, j    , xmm3 );
            (~C).store( i+SIMDSIZE*3UL, j    , xmm4 );
            (~C).store( i             , j+1UL, xmm5 );
            (~C).store( i+SIMDSIZE    , j+1UL, xmm6 );
            (~C).store( i+SIMDSIZE*2UL, j+1UL, xmm7 );
            (~C).store( i+SIMDSIZE*3UL, j+1UL, xmm8 );
         }

         if( j < N )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*4UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i             ,j) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE    ,j) );
            SIMDType xmm3( (~C).load(i+SIMDSIZE*2UL,j) );
            SIMDType xmm4( (~C).load(i+SIMDSIZE*3UL,j) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 += A.load(i             ,k) * b1;
               xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
               xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
               xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
            }

            (~C).store( i             , j, xmm1 );
            (~C).store( i+SIMDSIZE    , j, xmm2 );
            (~C).store( i+SIMDSIZE*2UL, j, xmm3 );
            (~C).store( i+SIMDSIZE*3UL, j, xmm4 );
         }
      }

      for( ; !LOW && !UPP && (i+SIMDSIZE*2UL) < ipos; i+=SIMDSIZE*3UL )
      {
         size_t j( 0UL );

         for( ; (j+2UL) <= N; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*3UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*3UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i             ,j    ) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE    ,j    ) );
            SIMDType xmm3( (~C).load(i+SIMDSIZE*2UL,j    ) );
            SIMDType xmm4( (~C).load(i             ,j+1UL) );
            SIMDType xmm5( (~C).load(i+SIMDSIZE    ,j+1UL) );
            SIMDType xmm6( (~C).load(i+SIMDSIZE*2UL,j+1UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i             ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
               const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a3 * b1;
               xmm4 += a1 * b2;
               xmm5 += a2 * b2;
               xmm6 += a3 * b2;
            }

            (~C).store( i             , j    , xmm1 );
            (~C).store( i+SIMDSIZE    , j    , xmm2 );
            (~C).store( i+SIMDSIZE*2UL, j    , xmm3 );
            (~C).store( i             , j+1UL, xmm4 );
            (~C).store( i+SIMDSIZE    , j+1UL, xmm5 );
            (~C).store( i+SIMDSIZE*2UL, j+1UL, xmm6 );
         }

         if( j < N )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*3UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i             ,j) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE    ,j) );
            SIMDType xmm3( (~C).load(i+SIMDSIZE*2UL,j) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 += A.load(i             ,k) * b1;
               xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
               xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
            }

            (~C).store( i             , j, xmm1 );
            (~C).store( i+SIMDSIZE    , j, xmm2 );
            (~C).store( i+SIMDSIZE*2UL, j, xmm3 );
         }
      }

      for( ; !( LOW && UPP ) && (i+SIMDSIZE) < ipos; i+=SIMDSIZE*2UL )
      {
         const size_t jend( LOW ? min(i+SIMDSIZE*2UL,N) : N );
         size_t j( UPP ? i : 0UL );

         for( ; (j+4UL) <= jend; j+=4UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i         ,j    ) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE,j    ) );
            SIMDType xmm3( (~C).load(i         ,j+1UL) );
            SIMDType xmm4( (~C).load(i+SIMDSIZE,j+1UL) );
            SIMDType xmm5( (~C).load(i         ,j+2UL) );
            SIMDType xmm6( (~C).load(i+SIMDSIZE,j+2UL) );
            SIMDType xmm7( (~C).load(i         ,j+3UL) );
            SIMDType xmm8( (~C).load(i+SIMDSIZE,j+3UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i         ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               const SIMDType b3( set( B(k,j+2UL) ) );
               const SIMDType b4( set( B(k,j+3UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a1 * b2;
               xmm4 += a2 * b2;
               xmm5 += a1 * b3;
               xmm6 += a2 * b3;
               xmm7 += a1 * b4;
               xmm8 += a2 * b4;
            }

            (~C).store( i         , j    , xmm1 );
            (~C).store( i+SIMDSIZE, j    , xmm2 );
            (~C).store( i         , j+1UL, xmm3 );
            (~C).store( i+SIMDSIZE, j+1UL, xmm4 );
            (~C).store( i         , j+2UL, xmm5 );
            (~C).store( i+SIMDSIZE, j+2UL, xmm6 );
            (~C).store( i         , j+3UL, xmm7 );
            (~C).store( i+SIMDSIZE, j+3UL, xmm8 );
         }

         for( ; (j+3UL) <= jend; j+=3UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i         ,j    ) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE,j    ) );
            SIMDType xmm3( (~C).load(i         ,j+1UL) );
            SIMDType xmm4( (~C).load(i+SIMDSIZE,j+1UL) );
            SIMDType xmm5( (~C).load(i         ,j+2UL) );
            SIMDType xmm6( (~C).load(i+SIMDSIZE,j+2UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i         ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               const SIMDType b3( set( B(k,j+2UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a1 * b2;
               xmm4 += a2 * b2;
               xmm5 += a1 * b3;
               xmm6 += a2 * b3;
            }

            (~C).store( i         , j    , xmm1 );
            (~C).store( i+SIMDSIZE, j    , xmm2 );
            (~C).store( i         , j+1UL, xmm3 );
            (~C).store( i+SIMDSIZE, j+1UL, xmm4 );
            (~C).store( i         , j+2UL, xmm5 );
            (~C).store( i+SIMDSIZE, j+2UL, xmm6 );
         }

         for( ; (j+2UL) <= jend; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i         ,j    ) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE,j    ) );
            SIMDType xmm3( (~C).load(i         ,j+1UL) );
            SIMDType xmm4( (~C).load(i+SIMDSIZE,j+1UL) );
            SIMDType xmm5, xmm6, xmm7, xmm8;
            size_t k( kbegin );

            for( ; (k+2UL) < kend; k+=2UL ) {
               const SIMDType a1( A.load(i         ,k    ) );
               const SIMDType a2( A.load(i+SIMDSIZE,k    ) );
               const SIMDType a3( A.load(i         ,k+1UL) );
               const SIMDType a4( A.load(i+SIMDSIZE,k+1UL) );
               const SIMDType b1( set( B(k    ,j    ) ) );
               const SIMDType b2( set( B(k    ,j+1UL) ) );
               const SIMDType b3( set( B(k+1UL,j    ) ) );
               const SIMDType b4( set( B(k+1UL,j+1UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a1 * b2;
               xmm4 += a2 * b2;
               xmm5 += a3 * b3;
               xmm6 += a4 * b3;
               xmm7 += a3 * b4;
               xmm8 += a4 * b4;
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i         ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a1 * b2;
               xmm4 += a2 * b2;
            }

            (~C).store( i         , j    , xmm1+xmm5 );
            (~C).store( i+SIMDSIZE, j    , xmm2+xmm6 );
            (~C).store( i         , j+1UL, xmm3+xmm7 );
            (~C).store( i+SIMDSIZE, j+1UL, xmm4+xmm8 );
         }

         if( j < jend )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*2UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i         ,j) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE,j) );
            SIMDType xmm3, xmm4;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType b1( set( B(k    ,j) ) );
               const SIMDType b2( set( B(k+1UL,j) ) );
               xmm1 += A.load(i         ,k    ) * b1;
               xmm2 += A.load(i+SIMDSIZE,k    ) * b1;
               xmm3 += A.load(i         ,k+1UL) * b2;
               xmm4 += A.load(i+SIMDSIZE,k+1UL) * b2;
            }

            for( ; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 += A.load(i         ,k) * b1;
               xmm2 += A.load(i+SIMDSIZE,k) * b1;
            }

            (~C).store( i         , j, xmm1+xmm3 );
            (~C).store( i+SIMDSIZE, j, xmm2+xmm4 );
         }
      }

      for( ; i<ipos; i+=SIMDSIZE )
      {
         const size_t jend( LOW && UPP ? min(i+SIMDSIZE,N) : N );
         size_t j( UPP ? i : 0UL );

         for( ; (j+4UL) <= jend; j+=4UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL )
                               :( K ) );

            SIMDType xmm1( (~C).load(i,j    ) );
            SIMDType xmm2( (~C).load(i,j+1UL) );
            SIMDType xmm3( (~C).load(i,j+2UL) );
            SIMDType xmm4( (~C).load(i,j+3UL) );
            SIMDType xmm5, xmm6, xmm7, xmm8;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i,k    ) );
               const SIMDType a2( A.load(i,k+1UL) );
               xmm1 += a1 * set( B(k    ,j    ) );
               xmm2 += a1 * set( B(k    ,j+1UL) );
               xmm3 += a1 * set( B(k    ,j+2UL) );
               xmm4 += a1 * set( B(k    ,j+3UL) );
               xmm5 += a2 * set( B(k+1UL,j    ) );
               xmm6 += a2 * set( B(k+1UL,j+1UL) );
               xmm7 += a2 * set( B(k+1UL,j+2UL) );
               xmm8 += a2 * set( B(k+1UL,j+3UL) );
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i,k) );
               xmm1 += a1 * set( B(k,j    ) );
               xmm2 += a1 * set( B(k,j+1UL) );
               xmm3 += a1 * set( B(k,j+2UL) );
               xmm4 += a1 * set( B(k,j+3UL) );
            }

            (~C).store( i, j    , xmm1+xmm5 );
            (~C).store( i, j+1UL, xmm2+xmm6 );
            (~C).store( i, j+2UL, xmm3+xmm7 );
            (~C).store( i, j+3UL, xmm4+xmm8 );
         }

         for( ; (j+3UL) <= jend; j+=3UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL )
                               :( K ) );

            SIMDType xmm1( (~C).load(i,j    ) );
            SIMDType xmm2( (~C).load(i,j+1UL) );
            SIMDType xmm3( (~C).load(i,j+2UL) );
            SIMDType xmm4, xmm5, xmm6;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i,k    ) );
               const SIMDType a2( A.load(i,k+1UL) );
               xmm1 += a1 * set( B(k    ,j    ) );
               xmm2 += a1 * set( B(k    ,j+1UL) );
               xmm3 += a1 * set( B(k    ,j+2UL) );
               xmm4 += a2 * set( B(k+1UL,j    ) );
               xmm5 += a2 * set( B(k+1UL,j+1UL) );
               xmm6 += a2 * set( B(k+1UL,j+2UL) );
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i,k) );
               xmm1 += a1 * set( B(k,j    ) );
               xmm2 += a1 * set( B(k,j+1UL) );
               xmm3 += a1 * set( B(k,j+2UL) );
            }

            (~C).store( i, j    , xmm1+xmm4 );
            (~C).store( i, j+1UL, xmm2+xmm5 );
            (~C).store( i, j+2UL, xmm3+xmm6 );
         }

         for( ; (j+2UL) <= jend; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL )
                               :( K ) );

            SIMDType xmm1( (~C).load(i,j    ) );
            SIMDType xmm2( (~C).load(i,j+1UL) );
            SIMDType xmm3, xmm4;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i,k    ) );
               const SIMDType a2( A.load(i,k+1UL) );
               xmm1 += a1 * set( B(k    ,j    ) );
               xmm2 += a1 * set( B(k    ,j+1UL) );
               xmm3 += a2 * set( B(k+1UL,j    ) );
               xmm4 += a2 * set( B(k+1UL,j+1UL) );
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i,k) );
               xmm1 += a1 * set( B(k,j    ) );
               xmm2 += a1 * set( B(k,j+1UL) );
            }

            (~C).store( i, j    , xmm1+xmm3 );
            (~C).store( i, j+1UL, xmm2+xmm4 );
         }

         if( j < jend )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );

            SIMDType xmm1( (~C).load(i,j) );
            SIMDType xmm2;
            size_t k( kbegin );

            for( ; (k+2UL) <= K; k+=2UL ) {
               xmm1 += A.load(i,k    ) * set( B(k    ,j) );
               xmm2 += A.load(i,k+1UL) * set( B(k+1UL,j) );
            }

            for( ; k<K; ++k ) {
               xmm1 += A.load(i,k) * set( B(k,j) );
            }

            (~C).store( i, j, xmm1+xmm2 );
         }
      }

      for( ; remainder && i<M; ++i )
      {
         const size_t jend( LOW ? i+1UL : N );
         size_t j( UPP ? i : 0UL );

         for( ; (j+2UL) <= jend; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL )
                               :( K ) );

            ElementType value1( (~C)(i,j    ) );
            ElementType value2( (~C)(i,j+1UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               value1 += A(i,k) * B(k,j    );
               value2 += A(i,k) * B(k,j+1UL);
            }

            (~C)(i,j    ) = value1;
            (~C)(i,j+1UL) = value2;
         }

         if( j < jend )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );

            ElementType value( (~C)(i,j) );

            for( size_t k=kbegin; k<K; ++k ) {
               value += A(i,k) * B(k,j);
            }

            (~C)(i,j) = value;
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (large matrices)******************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a large transpose dense matrix-transpose dense matrix
   //        multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the addition assignment of a transpose
   // dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectLargeAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectDefaultAddAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default addition assignment to dense matrices (large matrices)*******************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default addition assignment of a large transpose dense matrix-transpose
   //        dense matrix multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default addition assignment of a transpose dense
   // matrix-transpose dense matrix multiplication expression to a dense matrix. This kernel
   // is optimized for large matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectLargeAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      if( LOW )
         lmmm( C, A, B, ElementType(1), ElementType(1) );
      else if( UPP )
         ummm( C, A, B, ElementType(1), ElementType(1) );
      else
         mmm( C, A, B, ElementType(1), ElementType(1) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**BLAS-based addition assignment to dense matrices (default)**********************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default addition assignment of a transpose dense matrix-transpose dense matrix
   //        multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the addition assignment of a large
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseBlasKernel<MT3,MT4,MT5> >
      selectBlasAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectLargeAddAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**BLAS-based addition assignment to dense matrices********************************************
#if BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION
   /*! \cond BLAZE_INTERNAL */
   /*!\brief BLAS-based addition assignment of a transpose dense matrix-transpose dense matrix
   //        multiplication (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function performs the transpose dense matrix-transpose dense matrix multiplication
   // based on the according BLAS functionality.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseBlasKernel<MT3,MT4,MT5> >
      selectBlasAddAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      using ET = ElementType_<MT3>;

      if( IsTriangular<MT4>::value ) {
         ResultType_<MT3> tmp( serial( B ) );
         trmm( tmp, A, CblasLeft, ( IsLower<MT4>::value )?( CblasLower ):( CblasUpper ), ET(1) );
         addAssign( C, tmp );
      }
      else if( IsTriangular<MT5>::value ) {
         ResultType_<MT3> tmp( serial( A ) );
         trmm( tmp, B, CblasRight, ( IsLower<MT5>::value )?( CblasLower ):( CblasUpper ), ET(1) );
         addAssign( C, tmp );
      }
      else {
         gemm( C, A, B, ET(1), ET(1) );
      }
   }
   /*! \endcond */
#endif
   //**********************************************************************************************

   //**Restructuring addition assignment to row-major matrices*************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Restructuring addition assignment of a transpose dense matrix-transpose dense matrix
   //        multiplication to a row-major matrix (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side multiplication expression to be added.
   // \return void
   //
   // This function implements the symmetry-based restructuring addition assignment of a transpose
   // dense matrix-transpose dense matrix multiplication expression to a row-major matrix. Due to
   // the explicit application of the SFINAE principle this function can only be selected by the
   // compiler in case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      addAssign( Matrix<MT,false>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         addAssign( ~lhs, fwd( trans( rhs.lhs_ ) * trans( rhs.rhs_ ) ) );
      else if( IsSymmetric<MT1>::value )
         addAssign( ~lhs, fwd( trans( rhs.lhs_ ) * rhs.rhs_ ) );
      else
         addAssign( ~lhs, fwd( rhs.lhs_ * trans( rhs.rhs_ ) ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Addition assignment to sparse matrices******************************************************
   // No special implementation for the addition assignment to sparse matrices.
   //**********************************************************************************************

   //**Subtraction assignment to dense matrices****************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Subtraction assignment of a transpose dense matrix-transpose dense matrix
   //        multiplication to a dense matrix (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the performance optimized subtraction assignment of a transpose
   // dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      subAssign( DenseMatrix<MT,SO>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || rhs.lhs_.columns() == 0UL ) {
         return;
      }

      LT A( serial( rhs.lhs_ ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( rhs.rhs_ ) );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      TDMatTDMatMultExpr::selectSubAssignKernel( ~lhs, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Subtraction assignment to dense matrices (kernel selection)*********************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Selection of the kernel for a subtraction assignment of a transpose dense matrix-
   //        transpose dense matrix multiplication to a dense matrix (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline void selectSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      if( ( IsDiagonal<MT4>::value ) ||
          ( !BLAZE_DEBUG_MODE && A.rows() <= SIMDSIZE*10UL ) ||
          ( C.rows() * C.columns() < TDMATTDMATMULT_THRESHOLD ) )
         selectSmallSubAssignKernel( C, A, B );
      else
         selectBlasSubAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (general/general)**************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a general transpose dense matrix-general transpose
   //        dense matrix multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default subtraction assignment of a general transpose dense
   // matrix-general transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, Not< IsDiagonal<MT5> > > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t kbegin( ( IsLower<MT5>::value )
                              ?( IsStrictlyLower<MT5>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t kend( ( IsUpper<MT5>::value )
                            ?( IsStrictlyUpper<MT5>::value ? j : j+1UL )
                            :( K ) );
         BLAZE_INTERNAL_ASSERT( kbegin <= kend, "Invalid loop indices detected" );

         for( size_t k=kbegin; k<kend; ++k )
         {
            const size_t ibegin( ( IsLower<MT4>::value )
                                 ?( ( IsStrictlyLower<MT4>::value )
                                    ?( LOW ? max(j,k+1UL) : k+1UL )
                                    :( LOW ? max(j,k) : k ) )
                                 :( LOW ? j : 0UL ) );
            const size_t iend( ( IsUpper<MT4>::value )
                               ?( ( IsStrictlyUpper<MT4>::value )
                                  ?( UPP ? min(j+1UL,k) : k )
                                  :( UPP ? min(j,k)+1UL : k+1UL ) )
                               :( UPP ? j+1UL : M ) );

            if( ( LOW || UPP ) && ( ibegin >= iend ) ) continue;
            BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

            const size_t inum( iend - ibegin );
            const size_t ipos( ibegin + ( inum & size_t(-2) ) );

            for( size_t i=ibegin; i<ipos; i+=2UL ) {
               C(i    ,j) -= A(i    ,k) * B(k,j);
               C(i+1UL,j) -= A(i+1UL,k) * B(k,j);
            }
            if( ipos < iend ) {
               C(ipos,j) -= A(ipos,k) * B(k,j);
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (general/diagonal)*************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a general transpose dense matrix-diagonal transpose
   //        dense matrix multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default subtraction assignment of a general transpose dense
   // matrix-diagonal transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, IsDiagonal<MT5> > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t ibegin( ( IsLower<MT4>::value )
                              ?( IsStrictlyLower<MT4>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t iend( ( IsUpper<MT4>::value )
                            ?( IsStrictlyUpper<MT4>::value ? j : j+1UL )
                            :( M ) );
         BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

         const size_t inum( iend - ibegin );
         const size_t ipos( ibegin + ( inum & size_t(-2) ) );

         for( size_t i=ibegin; i<ipos; i+=2UL ) {
            C(i    ,j) -= A(i    ,j) * B(j,j);
            C(i+1UL,j) -= A(i+1UL,j) * B(j,j);
         }
         if( ipos < iend ) {
            C(ipos,j) -= A(ipos,j) * B(j,j);
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (diagonal/general)*************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a diagonal transpose dense matrix-general transpose
   //        dense matrix multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default subtraction assignment of a diagonal transpose dense
   // matrix-general transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< IsDiagonal<MT4>, Not< IsDiagonal<MT5> > > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t ibegin( ( IsLower<MT5>::value )
                              ?( IsStrictlyLower<MT5>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t iend( ( IsUpper<MT5>::value )
                            ?( IsStrictlyUpper<MT5>::value ? j : j+1UL )
                            :( M ) );
         BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

         const size_t inum( iend - ibegin );
         const size_t ipos( ibegin + ( inum & size_t(-2) ) );

         for( size_t i=ibegin; i<ipos; i+=2UL ) {
            C(i    ,j) -= A(i    ,i    ) * B(i    ,j);
            C(i+1UL,j) -= A(i+1UL,i+1UL) * B(i+1UL,j);
         }
         if( ipos < iend ) {
            C(ipos,j) -= A(ipos,ipos) * B(ipos,j);
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (diagonal/diagonal)************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a diagonal transpose dense matrix-diagonal transpose
   //        dense matrix multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the default subtraction assignment of a diagonal transpose dense
   // matrix-diagonal transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< And< IsDiagonal<MT4>, IsDiagonal<MT5> > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      for( size_t i=0UL; i<A.rows(); ++i ) {
         C(i,i) -= A(i,i) * B(i,i);
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (small matrices)***************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a small transpose dense matrix-transpose dense
   //        matrix multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the subtraction assignment of a
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectDefaultSubAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default subtraction assignment to row-major dense matrices (small matrices)******
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default subtraction assignment of a small transpose dense matrix-transpose
   //        dense matrix multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default subtraction assignment of a transpose dense
   // matrix-transpose dense matrix multiplication expression to a row-major dense matrix. This
   // kernel is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallSubAssignKernel( DenseMatrix<MT3,false>& C, const MT4& A, const MT5& B )
   {
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT4 );
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT5 );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType_<MT4> );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType_<MT5> );

      const ForwardFunctor fwd;

      if( IsResizable<MT4>::value && !IsResizable<MT5>::value ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         subAssign( ~C, fwd( A * tmp ) );
      }
      else if( !IsResizable<MT4>::value && IsResizable<MT5>::value ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         subAssign( ~C, fwd( tmp * B ) );
      }
      else if( B.rows() * B.columns() <= A.rows() * A.columns() ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         subAssign( ~C, fwd( A * tmp ) );
      }
      else {
         const OppositeType_<MT4> tmp( serial( A ) );
         subAssign( ~C, fwd( tmp * B ) );
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default subtraction assignment to column-major dense matrices (small matrices)***
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default subtraction assignment of a small transpose dense matrix-transpose
   //        dense matrix multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default subtraction assignment of a transpose dense
   // matrix-transpose dense matrix multiplication expression to a column-major dense matrix.
   // This kernel is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectSmallSubAssignKernel( DenseMatrix<MT3,true>& C, const MT4& A, const MT5& B )
   {
      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT4>::value );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      const size_t ipos( remainder ? ( M & size_t(-SIMDSIZE) ) : M );
      BLAZE_INTERNAL_ASSERT( !remainder || ( M - ( M % SIMDSIZE ) ) == ipos, "Invalid end calculation" );

      size_t i( 0UL );

      if( IsIntegral<ElementType>::value )
      {
         for( ; !LOW && !UPP && (i+SIMDSIZE*7UL) < ipos; i+=SIMDSIZE*8UL ) {
            for( size_t j=0UL; j<N; ++j )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*8UL, K, ( IsStrictlyUpper<MT5>::value ? j : j+1UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j : j+1UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*8UL, K ) : K ) );

               SIMDType xmm1( (~C).load(i             ,j) );
               SIMDType xmm2( (~C).load(i+SIMDSIZE    ,j) );
               SIMDType xmm3( (~C).load(i+SIMDSIZE*2UL,j) );
               SIMDType xmm4( (~C).load(i+SIMDSIZE*3UL,j) );
               SIMDType xmm5( (~C).load(i+SIMDSIZE*4UL,j) );
               SIMDType xmm6( (~C).load(i+SIMDSIZE*5UL,j) );
               SIMDType xmm7( (~C).load(i+SIMDSIZE*6UL,j) );
               SIMDType xmm8( (~C).load(i+SIMDSIZE*7UL,j) );

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType b1( set( B(k,j) ) );
                  xmm1 -= A.load(i             ,k) * b1;
                  xmm2 -= A.load(i+SIMDSIZE    ,k) * b1;
                  xmm3 -= A.load(i+SIMDSIZE*2UL,k) * b1;
                  xmm4 -= A.load(i+SIMDSIZE*3UL,k) * b1;
                  xmm5 -= A.load(i+SIMDSIZE*4UL,k) * b1;
                  xmm6 -= A.load(i+SIMDSIZE*5UL,k) * b1;
                  xmm7 -= A.load(i+SIMDSIZE*6UL,k) * b1;
                  xmm8 -= A.load(i+SIMDSIZE*7UL,k) * b1;
               }

               (~C).store( i             , j, xmm1 );
               (~C).store( i+SIMDSIZE    , j, xmm2 );
               (~C).store( i+SIMDSIZE*2UL, j, xmm3 );
               (~C).store( i+SIMDSIZE*3UL, j, xmm4 );
               (~C).store( i+SIMDSIZE*4UL, j, xmm5 );
               (~C).store( i+SIMDSIZE*5UL, j, xmm6 );
               (~C).store( i+SIMDSIZE*6UL, j, xmm7 );
               (~C).store( i+SIMDSIZE*7UL, j, xmm8 );
            }
         }
      }

      for( ; !LOW && !UPP && (i+SIMDSIZE*4UL) < ipos; i+=SIMDSIZE*5UL )
      {
         size_t j( 0UL );

         for( ; (j+2UL) <= N; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*5UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*5UL, K ) : K ) );

            SIMDType xmm1 ( (~C).load(i             ,j    ) );
            SIMDType xmm2 ( (~C).load(i+SIMDSIZE    ,j    ) );
            SIMDType xmm3 ( (~C).load(i+SIMDSIZE*2UL,j    ) );
            SIMDType xmm4 ( (~C).load(i+SIMDSIZE*3UL,j    ) );
            SIMDType xmm5 ( (~C).load(i+SIMDSIZE*4UL,j    ) );
            SIMDType xmm6 ( (~C).load(i             ,j+1UL) );
            SIMDType xmm7 ( (~C).load(i+SIMDSIZE    ,j+1UL) );
            SIMDType xmm8 ( (~C).load(i+SIMDSIZE*2UL,j+1UL) );
            SIMDType xmm9 ( (~C).load(i+SIMDSIZE*3UL,j+1UL) );
            SIMDType xmm10( (~C).load(i+SIMDSIZE*4UL,j+1UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i             ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
               const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
               const SIMDType a4( A.load(i+SIMDSIZE*3UL,k) );
               const SIMDType a5( A.load(i+SIMDSIZE*4UL,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1  -= a1 * b1;
               xmm2  -= a2 * b1;
               xmm3  -= a3 * b1;
               xmm4  -= a4 * b1;
               xmm5  -= a5 * b1;
               xmm6  -= a1 * b2;
               xmm7  -= a2 * b2;
               xmm8  -= a3 * b2;
               xmm9  -= a4 * b2;
               xmm10 -= a5 * b2;
            }

            (~C).store( i             , j    , xmm1  );
            (~C).store( i+SIMDSIZE    , j    , xmm2  );
            (~C).store( i+SIMDSIZE*2UL, j    , xmm3  );
            (~C).store( i+SIMDSIZE*3UL, j    , xmm4  );
            (~C).store( i+SIMDSIZE*4UL, j    , xmm5  );
            (~C).store( i             , j+1UL, xmm6  );
            (~C).store( i+SIMDSIZE    , j+1UL, xmm7  );
            (~C).store( i+SIMDSIZE*2UL, j+1UL, xmm8  );
            (~C).store( i+SIMDSIZE*3UL, j+1UL, xmm9  );
            (~C).store( i+SIMDSIZE*4UL, j+1UL, xmm10 );
         }

         if( j < N )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*5UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i             ,j) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE    ,j) );
            SIMDType xmm3( (~C).load(i+SIMDSIZE*2UL,j) );
            SIMDType xmm4( (~C).load(i+SIMDSIZE*3UL,j) );
            SIMDType xmm5( (~C).load(i+SIMDSIZE*4UL,j) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 -= A.load(i             ,k) * b1;
               xmm2 -= A.load(i+SIMDSIZE    ,k) * b1;
               xmm3 -= A.load(i+SIMDSIZE*2UL,k) * b1;
               xmm4 -= A.load(i+SIMDSIZE*3UL,k) * b1;
               xmm5 -= A.load(i+SIMDSIZE*4UL,k) * b1;
            }

            (~C).store( i             , j, xmm1 );
            (~C).store( i+SIMDSIZE    , j, xmm2 );
            (~C).store( i+SIMDSIZE*2UL, j, xmm3 );
            (~C).store( i+SIMDSIZE*3UL, j, xmm4 );
            (~C).store( i+SIMDSIZE*4UL, j, xmm5 );
         }
      }

      for( ; !LOW && !UPP && (i+SIMDSIZE*3UL) < ipos; i+=SIMDSIZE*4UL )
      {
         size_t j( 0UL );

         for( ; (j+2UL) <= N; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*4UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*4UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i             ,j    ) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE    ,j    ) );
            SIMDType xmm3( (~C).load(i+SIMDSIZE*2UL,j    ) );
            SIMDType xmm4( (~C).load(i+SIMDSIZE*3UL,j    ) );
            SIMDType xmm5( (~C).load(i             ,j+1UL) );
            SIMDType xmm6( (~C).load(i+SIMDSIZE    ,j+1UL) );
            SIMDType xmm7( (~C).load(i+SIMDSIZE*2UL,j+1UL) );
            SIMDType xmm8( (~C).load(i+SIMDSIZE*3UL,j+1UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i             ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
               const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
               const SIMDType a4( A.load(i+SIMDSIZE*3UL,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1 -= a1 * b1;
               xmm2 -= a2 * b1;
               xmm3 -= a3 * b1;
               xmm4 -= a4 * b1;
               xmm5 -= a1 * b2;
               xmm6 -= a2 * b2;
               xmm7 -= a3 * b2;
               xmm8 -= a4 * b2;
            }

            (~C).store( i             , j    , xmm1 );
            (~C).store( i+SIMDSIZE    , j    , xmm2 );
            (~C).store( i+SIMDSIZE*2UL, j    , xmm3 );
            (~C).store( i+SIMDSIZE*3UL, j    , xmm4 );
            (~C).store( i             , j+1UL, xmm5 );
            (~C).store( i+SIMDSIZE    , j+1UL, xmm6 );
            (~C).store( i+SIMDSIZE*2UL, j+1UL, xmm7 );
            (~C).store( i+SIMDSIZE*3UL, j+1UL, xmm8 );
         }

         if( j < N )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*4UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i             ,j) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE    ,j) );
            SIMDType xmm3( (~C).load(i+SIMDSIZE*2UL,j) );
            SIMDType xmm4( (~C).load(i+SIMDSIZE*3UL,j) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 -= A.load(i             ,k) * b1;
               xmm2 -= A.load(i+SIMDSIZE    ,k) * b1;
               xmm3 -= A.load(i+SIMDSIZE*2UL,k) * b1;
               xmm4 -= A.load(i+SIMDSIZE*3UL,k) * b1;
            }

            (~C).store( i             , j, xmm1 );
            (~C).store( i+SIMDSIZE    , j, xmm2 );
            (~C).store( i+SIMDSIZE*2UL, j, xmm3 );
            (~C).store( i+SIMDSIZE*3UL, j, xmm4 );
         }
      }

      for( ; !LOW && !UPP && (i+SIMDSIZE*2UL) < ipos; i+=SIMDSIZE*3UL )
      {
         size_t j( 0UL );

         for( ; (j+2UL) <= N; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*3UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*3UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i             ,j    ) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE    ,j    ) );
            SIMDType xmm3( (~C).load(i+SIMDSIZE*2UL,j    ) );
            SIMDType xmm4( (~C).load(i             ,j+1UL) );
            SIMDType xmm5( (~C).load(i+SIMDSIZE    ,j+1UL) );
            SIMDType xmm6( (~C).load(i+SIMDSIZE*2UL,j+1UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i             ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
               const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1 -= a1 * b1;
               xmm2 -= a2 * b1;
               xmm3 -= a3 * b1;
               xmm4 -= a1 * b2;
               xmm5 -= a2 * b2;
               xmm6 -= a3 * b2;
            }

            (~C).store( i             , j    , xmm1 );
            (~C).store( i+SIMDSIZE    , j    , xmm2 );
            (~C).store( i+SIMDSIZE*2UL, j    , xmm3 );
            (~C).store( i             , j+1UL, xmm4 );
            (~C).store( i+SIMDSIZE    , j+1UL, xmm5 );
            (~C).store( i+SIMDSIZE*2UL, j+1UL, xmm6 );
         }

         if( j < N )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*3UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i             ,j) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE    ,j) );
            SIMDType xmm3( (~C).load(i+SIMDSIZE*2UL,j) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 -= A.load(i             ,k) * b1;
               xmm2 -= A.load(i+SIMDSIZE    ,k) * b1;
               xmm3 -= A.load(i+SIMDSIZE*2UL,k) * b1;
            }

            (~C).store( i             , j, xmm1 );
            (~C).store( i+SIMDSIZE    , j, xmm2 );
            (~C).store( i+SIMDSIZE*2UL, j, xmm3 );
         }
      }

      for( ; !( LOW && UPP ) && (i+SIMDSIZE) < ipos; i+=SIMDSIZE*2UL )
      {
         const size_t jend( LOW ? min(i+SIMDSIZE*2UL,N) : N );
         size_t j( UPP ? i : 0UL );

         for( ; (j+4UL) <= jend; j+=4UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i         ,j    ) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE,j    ) );
            SIMDType xmm3( (~C).load(i         ,j+1UL) );
            SIMDType xmm4( (~C).load(i+SIMDSIZE,j+1UL) );
            SIMDType xmm5( (~C).load(i         ,j+2UL) );
            SIMDType xmm6( (~C).load(i+SIMDSIZE,j+2UL) );
            SIMDType xmm7( (~C).load(i         ,j+3UL) );
            SIMDType xmm8( (~C).load(i+SIMDSIZE,j+3UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i         ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               const SIMDType b3( set( B(k,j+2UL) ) );
               const SIMDType b4( set( B(k,j+3UL) ) );
               xmm1 -= a1 * b1;
               xmm2 -= a2 * b1;
               xmm3 -= a1 * b2;
               xmm4 -= a2 * b2;
               xmm5 -= a1 * b3;
               xmm6 -= a2 * b3;
               xmm7 -= a1 * b4;
               xmm8 -= a2 * b4;
            }

            (~C).store( i         , j    , xmm1 );
            (~C).store( i+SIMDSIZE, j    , xmm2 );
            (~C).store( i         , j+1UL, xmm3 );
            (~C).store( i+SIMDSIZE, j+1UL, xmm4 );
            (~C).store( i         , j+2UL, xmm5 );
            (~C).store( i+SIMDSIZE, j+2UL, xmm6 );
            (~C).store( i         , j+3UL, xmm7 );
            (~C).store( i+SIMDSIZE, j+3UL, xmm8 );
         }

         for( ; (j+3UL) <= jend; j+=3UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i         ,j    ) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE,j    ) );
            SIMDType xmm3( (~C).load(i         ,j+1UL) );
            SIMDType xmm4( (~C).load(i+SIMDSIZE,j+1UL) );
            SIMDType xmm5( (~C).load(i         ,j+2UL) );
            SIMDType xmm6( (~C).load(i+SIMDSIZE,j+2UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i         ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               const SIMDType b3( set( B(k,j+2UL) ) );
               xmm1 -= a1 * b1;
               xmm2 -= a2 * b1;
               xmm3 -= a1 * b2;
               xmm4 -= a2 * b2;
               xmm5 -= a1 * b3;
               xmm6 -= a2 * b3;
            }

            (~C).store( i         , j    , xmm1 );
            (~C).store( i+SIMDSIZE, j    , xmm2 );
            (~C).store( i         , j+1UL, xmm3 );
            (~C).store( i+SIMDSIZE, j+1UL, xmm4 );
            (~C).store( i         , j+2UL, xmm5 );
            (~C).store( i+SIMDSIZE, j+2UL, xmm6 );
         }

         for( ; (j+2UL) <= jend; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1( (~C).load(i         ,j    ) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE,j    ) );
            SIMDType xmm3( (~C).load(i         ,j+1UL) );
            SIMDType xmm4( (~C).load(i+SIMDSIZE,j+1UL) );
            SIMDType xmm5, xmm6, xmm7, xmm8;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i         ,k    ) );
               const SIMDType a2( A.load(i+SIMDSIZE,k    ) );
               const SIMDType a3( A.load(i         ,k+1UL) );
               const SIMDType a4( A.load(i+SIMDSIZE,k+1UL) );
               const SIMDType b1( set( B(k    ,j    ) ) );
               const SIMDType b2( set( B(k    ,j+1UL) ) );
               const SIMDType b3( set( B(k+1UL,j    ) ) );
               const SIMDType b4( set( B(k+1UL,j+1UL) ) );
               xmm1 -= a1 * b1;
               xmm2 -= a2 * b1;
               xmm3 -= a1 * b2;
               xmm4 -= a2 * b2;
               xmm5 -= a3 * b3;
               xmm6 -= a4 * b3;
               xmm7 -= a3 * b4;
               xmm8 -= a4 * b4;
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i         ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1 -= a1 * b1;
               xmm2 -= a2 * b1;
               xmm3 -= a1 * b2;
               xmm4 -= a2 * b2;
            }

            (~C).store( i         , j    , xmm1+xmm5 );
            (~C).store( i+SIMDSIZE, j    , xmm2+xmm6 );
            (~C).store( i         , j+1UL, xmm3+xmm7 );
            (~C).store( i+SIMDSIZE, j+1UL, xmm4+xmm8 );
         }

         if( j < jend )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*2UL, K ) ):( K ) );

            SIMDType xmm1( (~C).load(i         ,j) );
            SIMDType xmm2( (~C).load(i+SIMDSIZE,j) );
            SIMDType xmm3, xmm4;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType b1( set( B(k    ,j) ) );
               const SIMDType b2( set( B(k+1UL,j) ) );
               xmm1 -= A.load(i         ,k    ) * b1;
               xmm2 -= A.load(i+SIMDSIZE,k    ) * b1;
               xmm3 -= A.load(i         ,k+1UL) * b2;
               xmm4 -= A.load(i+SIMDSIZE,k+1UL) * b2;
            }

            for( ; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 -= A.load(i         ,k) * b1;
               xmm2 -= A.load(i+SIMDSIZE,k) * b1;
            }

            (~C).store( i         , j, xmm1+xmm3 );
            (~C).store( i+SIMDSIZE, j, xmm2+xmm4 );
         }
      }

      for( ; i<ipos; i+=SIMDSIZE )
      {
         const size_t jend( LOW && UPP ? min(i+SIMDSIZE,N) : N );
         size_t j( UPP ? i : 0UL );

         for( ; (j+4UL) <= jend; j+=4UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL )
                               :( K ) );

            SIMDType xmm1( (~C).load(i,j    ) );
            SIMDType xmm2( (~C).load(i,j+1UL) );
            SIMDType xmm3( (~C).load(i,j+2UL) );
            SIMDType xmm4( (~C).load(i,j+3UL) );
            SIMDType xmm5, xmm6, xmm7, xmm8;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i,k    ) );
               const SIMDType a2( A.load(i,k+1UL) );
               xmm1 -= a1 * set( B(k    ,j    ) );
               xmm2 -= a1 * set( B(k    ,j+1UL) );
               xmm3 -= a1 * set( B(k    ,j+2UL) );
               xmm4 -= a1 * set( B(k    ,j+3UL) );
               xmm5 -= a2 * set( B(k+1UL,j    ) );
               xmm6 -= a2 * set( B(k+1UL,j+1UL) );
               xmm7 -= a2 * set( B(k+1UL,j+2UL) );
               xmm8 -= a2 * set( B(k+1UL,j+3UL) );
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i,k) );
               xmm1 -= a1 * set( B(k,j    ) );
               xmm2 -= a1 * set( B(k,j+1UL) );
               xmm3 -= a1 * set( B(k,j+2UL) );
               xmm4 -= a1 * set( B(k,j+3UL) );
            }

            (~C).store( i, j    , xmm1+xmm5 );
            (~C).store( i, j+1UL, xmm2+xmm6 );
            (~C).store( i, j+2UL, xmm3+xmm7 );
            (~C).store( i, j+3UL, xmm4+xmm8 );
         }

         for( ; (j+3UL) <= jend; j+=3UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL )
                               :( K ) );

            SIMDType xmm1( (~C).load(i,j    ) );
            SIMDType xmm2( (~C).load(i,j+1UL) );
            SIMDType xmm3( (~C).load(i,j+2UL) );
            SIMDType xmm4, xmm5, xmm6;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i,k    ) );
               const SIMDType a2( A.load(i,k+1UL) );
               xmm1 -= a1 * set( B(k    ,j    ) );
               xmm2 -= a1 * set( B(k    ,j+1UL) );
               xmm3 -= a1 * set( B(k    ,j+2UL) );
               xmm4 -= a2 * set( B(k+1UL,j    ) );
               xmm5 -= a2 * set( B(k+1UL,j+1UL) );
               xmm6 -= a2 * set( B(k+1UL,j+2UL) );
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i,k) );
               xmm1 -= a1 * set( B(k,j    ) );
               xmm2 -= a1 * set( B(k,j+1UL) );
               xmm3 -= a1 * set( B(k,j+2UL) );
            }

            (~C).store( i, j    , xmm1+xmm4 );
            (~C).store( i, j+1UL, xmm2+xmm5 );
            (~C).store( i, j+2UL, xmm3+xmm6 );
         }

         for( ; (j+2UL) <= jend; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL )
                               :( K ) );

            SIMDType xmm1( (~C).load(i,j    ) );
            SIMDType xmm2( (~C).load(i,j+1UL) );
            SIMDType xmm3, xmm4;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i,k    ) );
               const SIMDType a2( A.load(i,k+1UL) );
               xmm1 -= a1 * set( B(k    ,j    ) );
               xmm2 -= a1 * set( B(k    ,j+1UL) );
               xmm3 -= a2 * set( B(k+1UL,j    ) );
               xmm4 -= a2 * set( B(k+1UL,j+1UL) );
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i,k) );
               xmm1 -= a1 * set( B(k,j    ) );
               xmm2 -= a1 * set( B(k,j+1UL) );
            }

            (~C).store( i, j    , xmm1+xmm3 );
            (~C).store( i, j+1UL, xmm2+xmm4 );
         }

         if( j < jend )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );

            SIMDType xmm1( (~C).load(i,j) );
            SIMDType xmm2;
            size_t k( kbegin );

            for( ; (k+2UL) <= K; k+=2UL ) {
               xmm1 -= A.load(i,k    ) * set( B(k    ,j) );
               xmm2 -= A.load(i,k+1UL) * set( B(k+1UL,j) );
            }

            for( ; k<K; ++k ) {
               xmm1 -= A.load(i,k) * set( B(k,j) );
            }

            (~C).store( i, j, xmm1+xmm2 );
         }
      }

      for( ; remainder && i<M; ++i )
      {
         const size_t jend( LOW ? i+1UL : N );
         size_t j( UPP ? i : 0UL );

         for( ; (j+2UL) <= jend; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL )
                               :( K ) );

            ElementType value1( (~C)(i,j    ) );
            ElementType value2( (~C)(i,j+1UL) );

            for( size_t k=kbegin; k<kend; ++k ) {
               value1 -= A(i,k) * B(k,j    );
               value2 -= A(i,k) * B(k,j+1UL);
            }

            (~C)(i,j    ) = value1;
            (~C)(i,j+1UL) = value2;
         }

         if( j < jend )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );

            ElementType value( (~C)(i,j) );

            for( size_t k=kbegin; k<K; ++k ) {
               value -= A(i,k) * B(k,j);
            }

            (~C)(i,j) = value;
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (large matrices)***************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a large transpose dense matrix-transpose dense
   //        matrix multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the subtraction assignment of a
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectLargeSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectDefaultSubAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Vectorized default subtraction assignment to dense matrices (large matrices)****************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Vectorized default subtraction assignment of a large transpose dense matrix-transpose
   //        dense matrix multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function implements the vectorized default subtraction assignment of a transpose dense
   // matrix-transpose dense matrix multiplication expression to a dense matrix. This kernel is
   // optimized for large matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5> >
      selectLargeSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      if( LOW )
         lmmm( C, A, B, ElementType(-1), ElementType(1) );
      else if( UPP )
         ummm( C, A, B, ElementType(-1), ElementType(1) );
      else
         mmm( C, A, B, ElementType(-1), ElementType(1) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**BLAS-based subtraction assignment to dense matrices (default)*******************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Default subtraction assignment of a transpose dense matrix-transpose dense matrix
   //        multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function relays to the default implementation of the subtraction assignment of a large
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline DisableIf_< UseBlasKernel<MT3,MT4,MT5> >
      selectBlasSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      selectLargeSubAssignKernel( C, A, B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**BLAS-based subraction assignment to dense matrices******************************************
#if BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION
   /*! \cond BLAZE_INTERNAL */
   /*!\brief BLAS-based subraction assignment of a transpose dense matrix-transpose dense matrix
   //        multiplication (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \return void
   //
   // This function performs the transpose dense matrix-transpose dense matrix multiplication
   // based on the according BLAS functionality.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5 >  // Type of the right-hand side matrix operand
   static inline EnableIf_< UseBlasKernel<MT3,MT4,MT5> >
      selectBlasSubAssignKernel( MT3& C, const MT4& A, const MT5& B )
   {
      using ET = ElementType_<MT3>;

      if( IsTriangular<MT4>::value ) {
         ResultType_<MT3> tmp( serial( B ) );
         trmm( tmp, A, CblasLeft, ( IsLower<MT4>::value )?( CblasLower ):( CblasUpper ), ET(1) );
         subAssign( C, tmp );
      }
      else if( IsTriangular<MT5>::value ) {
         ResultType_<MT3> tmp( serial( A ) );
         trmm( tmp, B, CblasRight, ( IsLower<MT5>::value )?( CblasLower ):( CblasUpper ), ET(1) );
         subAssign( C, tmp );
      }
      else {
         gemm( C, A, B, ET(-1), ET(1) );
      }
   }
   /*! \endcond */
#endif
   //**********************************************************************************************

   //**Restructuring subtraction assignment to row-major matrices**********************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Restructuring subtraction assignment of a transpose dense matrix-transpose dense
   //        matrix multiplication to a row-major matrix (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the symmetry-based restructuring subtraction assignment of a
   // transpose dense matrix-transpose dense matrix multiplication expression to a row-major
   // matrix. Due to the explicit application of the SFINAE principle this function can only
   // be selected by the compiler in case the symmetry of either of the two matrix operands
   // can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      subAssign( Matrix<MT,false>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         subAssign( ~lhs, fwd( trans( rhs.lhs_ ) * trans( rhs.rhs_ ) ) );
      else if( IsSymmetric<MT1>::value )
         subAssign( ~lhs, fwd( trans( rhs.lhs_ ) * rhs.rhs_ ) );
      else
         subAssign( ~lhs, fwd( rhs.lhs_ * trans( rhs.rhs_ ) ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Subtraction assignment to sparse matrices***************************************************
   // No special implementation for the subtraction assignment to sparse matrices.
   //**********************************************************************************************

   //**Schur product assignment to dense matrices**************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Schur product assignment of a transpose dense matrix-transpose dense matrix
   //        multiplication to a dense matrix (\f$ C\circ=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression for the Schur product.
   // \return void
   //
   // This function implements the performance optimized Schur product assignment of a transpose
   // dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline void schurAssign( DenseMatrix<MT,SO>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ResultType tmp( serial( rhs ) );
      schurAssign( ~lhs, tmp );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Multiplication assignment to dense matrices*************************************************
   // No special implementation for the multiplication assignment to dense matrices.
   //**********************************************************************************************

   //**Multiplication assignment to sparse matrices************************************************
   // No special implementation for the multiplication assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP assignment to dense matrices************************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief SMP assignment of a transpose dense matrix-transpose dense matrix multiplication to a
   //        dense matrix (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized SMP assignment of a transpose dense
   // matrix-transpose dense matrix multiplication expression to a dense matrix. Due to the
   // explicit application of the SFINAE principle, this function can only be selected by the
   // compiler in case either of the two matrix operands requires an intermediate evaluation
   // and no symmetry can be exploited.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAssign( DenseMatrix<MT,SO>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL ) {
         return;
      }
      else if( rhs.lhs_.columns() == 0UL ) {
         reset( ~lhs );
         return;
      }

      LT A( rhs.lhs_ );  // Evaluation of the left-hand side dense matrix operand
      RT B( rhs.rhs_ );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      smpAssign( ~lhs, A * B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**SMP assignment to sparse matrices***********************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief SMP assignment of a transpose dense matrix-transpose dense matrix multiplication
   //        to a sparse matrix (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side sparse matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized SMP assignment of a transpose dense
   // matrix-transpose dense matrix multiplication expression to a sparse matrix. Due to the
   // explicit application of the SFINAE principle, this function can only be selected by the
   // compiler in case either of the two matrix operands requires an intermediate evaluation
   // and no symmetry can be exploited.
   */
   template< typename MT  // Type of the target sparse matrix
           , bool SO >    // Storage order of the target sparse matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAssign( SparseMatrix<MT,SO>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      using TmpType = IfTrue_< SO, ResultType, OppositeType >;

      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MATRICES_MUST_HAVE_SAME_STORAGE_ORDER( MT, TmpType );
      BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( TmpType );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      const TmpType tmp( rhs );
      smpAssign( ~lhs, fwd( tmp ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Restructuring SMP assignment to row-major matrices******************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Restructuring SMP assignment of a transpose dense matrix-transpose dense matrix
   //        multiplication to a row-major matrix (\f$ C=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side multiplication expression to be assigned.
   // \return void
   //
   // This function implements the symmetry-based restructuring SMP assignment of a transpose
   // dense matrix-transpose dense matrix multiplication expression to a row-major matrix. Due
   // to the explicit application of the SFINAE principle this function can only be selected by
   // the compiler in case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      smpAssign( Matrix<MT,false>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         smpAssign( ~lhs, fwd( trans( rhs.lhs_ ) * trans( rhs.rhs_ ) ) );
      else if( IsSymmetric<MT1>::value )
         smpAssign( ~lhs, fwd( trans( rhs.lhs_ ) * rhs.rhs_ ) );
      else
         smpAssign( ~lhs, fwd( rhs.lhs_ * trans( rhs.rhs_ ) ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**SMP addition assignment to dense matrices***************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief SMP addition assignment of a transpose dense matrix-transpose dense matrix
   //        multiplication to a dense matrix (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be added.
   // \return void
   //
   // This function implements the performance optimized SMP addition assignment of a transpose
   // dense matrix-transpose dense matrix multiplication expression to a dense matrix. Due to
   // the explicit application of the SFINAE principle, this function can only be selected by
   // the compiler in case either of the two matrix operands requires an intermediate evaluation
   // and no symmetry can be exploited.
   */
   template< typename MT   // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAddAssign( DenseMatrix<MT,SO>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || rhs.lhs_.columns() == 0UL ) {
         return;
      }

      LT A( rhs.lhs_ );  // Evaluation of the left-hand side dense matrix operand
      RT B( rhs.rhs_ );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      smpAddAssign( ~lhs, A * B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Restructuring SMP addition assignment to row-major matrices*********************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Restructuring SMP addition assignment of a transpose dense matrix-transpose dense
   //        matrix multiplication to a row-major matrix (\f$ C+=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side multiplication expression to be added.
   // \return void
   //
   // This function implements the symmetry-based restructuring SMP addition assignment of a
   // transpose dense matrix-transpose dense matrix multiplication expression to a row-major
   // matrix. Due to the explicit application of the SFINAE principle this function can only be
   // selected by the compiler in case the symmetry of either of the two matrix operands can be
   // exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      smpAddAssign( Matrix<MT,false>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         smpAddAssign( ~lhs, fwd( trans( rhs.lhs_ ) * trans( rhs.rhs_ ) ) );
      else if( IsSymmetric<MT1>::value )
         smpAddAssign( ~lhs, fwd( trans( rhs.lhs_ ) * rhs.rhs_ ) );
      else
         smpAddAssign( ~lhs, fwd( rhs.lhs_ * trans( rhs.rhs_ ) ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**SMP addition assignment to sparse matrices**************************************************
   // No special implementation for the SMP addition assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP subtraction assignment to dense matrices************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief SMP subtraction assignment of a transpose dense matrix-transpose dense matrix
   //        multiplication to a dense matrix (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the performance optimized SMP subtraction assignment of a
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense
   // matrix. Due to the explicit application of the SFINAE principle, this function can
   // only be selected by the compiler in case either of the two matrix operands requires
   // an intermediate evaluation and no symmetry can be exploited.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpSubAssign( DenseMatrix<MT,SO>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || rhs.lhs_.columns() == 0UL ) {
         return;
      }

      LT A( rhs.lhs_ );  // Evaluation of the left-hand side dense matrix operand
      RT B( rhs.rhs_ );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

      smpSubAssign( ~lhs, A * B );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Restructuring SMP subtraction assignment to row-major matrices******************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Restructuring SMP subtraction assignment of a transpose dense matrix-transpose dense
   //        matrix multiplication to a row-major matrix (\f$ C-=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the symmetry-based restructuring SMP subtraction assignment of a
   // transpose dense matrix-transpose dense matrix multiplication expression to a row-major
   // matrix. Due to the explicit application of the SFINAE principle this function can only be
   // selected by the compiler in case the symmetry of either of the two matrix operands can be
   // exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      smpSubAssign( Matrix<MT,false>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         smpSubAssign( ~lhs, fwd( trans( rhs.lhs_ ) * trans( rhs.rhs_ ) ) );
      else if( IsSymmetric<MT1>::value )
         smpSubAssign( ~lhs, fwd( trans( rhs.lhs_ ) * rhs.rhs_ ) );
      else
         smpSubAssign( ~lhs, fwd( rhs.lhs_ * trans( rhs.rhs_ ) ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**SMP subtraction assignment to sparse matrices***********************************************
   // No special implementation for the SMP subtraction assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP Schur product assignment to dense matrices**********************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief SMP Schur product assignment of a transpose dense matrix-transpose dense matrix
   //        multiplication to a dense matrix (\f$ C\circ=A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression for the Schur product.
   // \return void
   //
   // This function implements the performance optimized SMP Schur product assignment of a
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense
   // matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline void smpSchurAssign( DenseMatrix<MT,SO>& lhs, const TDMatTDMatMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ResultType tmp( rhs );
      smpSchurAssign( ~lhs, tmp );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**SMP Schur product assignment to sparse matrices*********************************************
   // No special implementation for the SMP Schur product assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP multiplication assignment to dense matrices*********************************************
   // No special implementation for the SMP multiplication assignment to dense matrices.
   //**********************************************************************************************

   //**SMP multiplication assignment to sparse matrices********************************************
   // No special implementation for the SMP multiplication assignment to sparse matrices.
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   /*! \cond BLAZE_INTERNAL */
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_FORM_VALID_MATMATMULTEXPR( MT1, MT2 );
   /*! \endcond */
   //**********************************************************************************************
};
//*************************************************************************************************




//=================================================================================================
//
//  DMATSCALARMULTEXPR SPECIALIZATION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Expression object for scaled transpose dense matrix-transpose dense matrix multiplications.
// \ingroup dense_matrix_expression
//
// This specialization of the DMatScalarMultExpr class represents the compile time expression
// for scaled multiplications between two column-major dense matrices.
*/
template< typename MT1   // Type of the left-hand side dense matrix
        , typename MT2   // Type of the right-hand side dense matrix
        , bool SF        // Symmetry flag
        , bool HF        // Hermitian flag
        , bool LF        // Lower flag
        , bool UF        // Upper flag
        , typename ST >  // Type of the right-hand side scalar value
class DMatScalarMultExpr< TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF>, ST, true >
   : public MatScalarMultExpr< DenseMatrix< DMatScalarMultExpr< TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF>, ST, true >, true > >
   , private Computation
{
 private:
   //**Type definitions****************************************************************************
   //! Type of the dense matrix multiplication expression.
   using MMM = TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF>;

   using RES = ResultType_<MMM>;     //!< Result type of the dense matrix multiplication expression.
   using RT1 = ResultType_<MT1>;     //!< Result type of the left-hand side dense matrix expression.
   using RT2 = ResultType_<MT2>;     //!< Result type of the right-hand side dense matrix expression.
   using ET1 = ElementType_<RT1>;    //!< Element type of the left-hand side dense matrix expression.
   using ET2 = ElementType_<RT2>;    //!< Element type of the right-hand side dense matrix expression.
   using CT1 = CompositeType_<MT1>;  //!< Composite type of the left-hand side dense matrix expression.
   using CT2 = CompositeType_<MT2>;  //!< Composite type of the right-hand side dense matrix expression.
   //**********************************************************************************************

   //**********************************************************************************************
   //! Compilation switch for the composite type of the left-hand side dense matrix expression.
   enum : bool { evaluateLeft = IsComputation<MT1>::value || RequiresEvaluation<MT1>::value };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Compilation switch for the composite type of the right-hand side dense matrix expression.
   enum : bool { evaluateRight = IsComputation<MT2>::value || RequiresEvaluation<MT2>::value };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Compilation switches for the kernel generation.
   enum : bool {
      SYM  = ( SF && !( HF || LF || UF )    ),  //!< Flag for symmetric matrices.
      HERM = ( HF && !( LF || UF )          ),  //!< Flag for Hermitian matrices.
      LOW  = ( LF || ( ( SF || HF ) && UF ) ),  //!< Flag for lower matrices.
      UPP  = ( UF || ( ( SF || HF ) && LF ) )   //!< Flag for upper matrices.
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! The CanExploitSymmetry struct is a helper struct for the selection of the optimal
       evaluation strategy. In case the target matrix is row-major and either of the two
       matrix operands is symmetric, \a value is set to 1 and an optimized evaluation
       strategy is selected. Otherwise \a value is set to 0 and the default strategy is
       chosen. */
   template< typename T1, typename T2, typename T3 >
   struct CanExploitSymmetry {
      enum : bool { value = IsRowMajorMatrix<T1>::value &&
                            ( IsSymmetric<T2>::value || IsSymmetric<T3>::value ) };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! The IsEvaluationRequired struct is a helper struct for the selection of the parallel
       evaluation strategy. In case either of the two matrix operands requires an intermediate
       evaluation, the nested \value will be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3 >
   struct IsEvaluationRequired {
      enum : bool { value = ( evaluateLeft || evaluateRight ) &&
                            !CanExploitSymmetry<T1,T2,T3>::value };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! In case the types of all three involved matrices and the scalar type are suited for a BLAS
       kernel, the nested \a value will be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3, typename T4 >
   struct UseBlasKernel {
      enum : bool { value = BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION &&
                            !SYM && !HERM && !LOW && !UPP &&
                            IsContiguous<T1>::value && HasMutableDataAccess<T1>::value &&
                            IsContiguous<T2>::value && HasConstDataAccess<T2>::value &&
                            IsContiguous<T3>::value && HasConstDataAccess<T3>::value &&
                            !IsDiagonal<T2>::value && !IsDiagonal<T3>::value &&
                            T1::simdEnabled && T2::simdEnabled && T3::simdEnabled &&
                            IsBLASCompatible< ElementType_<T1> >::value &&
                            IsBLASCompatible< ElementType_<T2> >::value &&
                            IsBLASCompatible< ElementType_<T3> >::value &&
                            IsSame< ElementType_<T1>, ElementType_<T2> >::value &&
                            IsSame< ElementType_<T1>, ElementType_<T3> >::value &&
                            !( IsBuiltin< ElementType_<T1> >::value && IsComplex<T4>::value ) };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! In case all four involved data types are suited for a vectorized computation of the
       matrix multiplication, the nested \value will be set to 1, otherwise it will be 0. */
   template< typename T1, typename T2, typename T3, typename T4 >
   struct UseVectorizedDefaultKernel {
      enum : bool { value = useOptimizedKernels &&
                            !IsDiagonal<T2>::value &&
                            T1::simdEnabled && T2::simdEnabled && T3::simdEnabled &&
                            IsSIMDCombinable< ElementType_<T1>
                                            , ElementType_<T2>
                                            , ElementType_<T3>
                                            , T4 >::value &&
                            HasSIMDAdd< ElementType_<T2>, ElementType_<T2> >::value &&
                            HasSIMDMult< ElementType_<T3>, ElementType_<T3> >::value };
   };
   //**********************************************************************************************

   //**********************************************************************************************
   //! Type of the functor for forwarding an expression to another assign kernel.
   /*! In case a temporary matrix needs to be created, this functor is used to forward the
       resulting expression to another assign kernel. */
   using ForwardFunctor = IfTrue_< HERM
                                 , DeclHerm
                                 , IfTrue_< SYM
                                          , DeclSym
                                          , IfTrue_< LOW
                                                   , IfTrue_< UPP
                                                            , DeclDiag
                                                            , DeclLow >
                                                   , IfTrue_< UPP
                                                            , DeclUpp
                                                            , Noop > > > >;
   //**********************************************************************************************

 public:
   //**Type definitions****************************************************************************
   using This          = DMatScalarMultExpr<MMM,ST,true>;  //!< Type of this DMatScalarMultExpr instance.
   using ResultType    = MultTrait_<RES,ST>;               //!< Result type for expression template evaluations.
   using OppositeType  = OppositeType_<ResultType>;        //!< Result type with opposite storage order for expression template evaluations.
   using TransposeType = TransposeType_<ResultType>;       //!< Transpose type for expression template evaluations.
   using ElementType   = ElementType_<ResultType>;         //!< Resulting element type.
   using SIMDType      = SIMDTrait_<ElementType>;          //!< Resulting SIMD element type.
   using ReturnType    = const ElementType;                //!< Return type for expression template evaluations.
   using CompositeType = const ResultType;                 //!< Data type for composite expression templates.

   //! Composite type of the left-hand side dense matrix expression.
   using LeftOperand = const TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF>;

   //! Composite type of the right-hand side scalar value.
   using RightOperand = ST;

   //! Type for the assignment of the left-hand side dense matrix operand.
   using LT = IfTrue_< evaluateLeft, const RT1, CT1 >;

   //! Type for the assignment of the right-hand side dense matrix operand.
   using RT = IfTrue_< evaluateRight, const RT2, CT2 >;
   //**********************************************************************************************

   //**Compilation flags***************************************************************************
   //! Compilation switch for the expression template evaluation strategy.
   enum : bool { simdEnabled = !IsDiagonal<MT1>::value &&
                               MT1::simdEnabled && MT2::simdEnabled &&
                               IsSIMDCombinable<ET1,ET2,ST>::value &&
                               HasSIMDAdd<ET1,ET2>::value &&
                               HasSIMDMult<ET1,ET2>::value };

   //! Compilation switch for the expression template assignment strategy.
   enum : bool { smpAssignable = !evaluateLeft  && MT1::smpAssignable &&
                                 !evaluateRight && MT2::smpAssignable };
   //**********************************************************************************************

   //**SIMD properties*****************************************************************************
   //! The number of elements packed within a single SIMD element.
   enum : size_t { SIMDSIZE = SIMDTrait<ElementType>::size };
   //**********************************************************************************************

   //**Constructor*********************************************************************************
   /*!\brief Constructor for the DMatScalarMultExpr class.
   //
   // \param matrix The left-hand side dense matrix of the multiplication expression.
   // \param scalar The right-hand side scalar of the multiplication expression.
   */
   explicit inline DMatScalarMultExpr( const MMM& matrix, ST scalar )
      : matrix_( matrix )  // Left-hand side dense matrix of the multiplication expression
      , scalar_( scalar )  // Right-hand side scalar of the multiplication expression
   {}
   //**********************************************************************************************

   //**Access operator*****************************************************************************
   /*!\brief 2D-access to the matrix elements.
   //
   // \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
   // \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
   // \return The resulting value.
   */
   inline ReturnType operator()( size_t i, size_t j ) const {
      BLAZE_INTERNAL_ASSERT( i < matrix_.rows()   , "Invalid row access index"    );
      BLAZE_INTERNAL_ASSERT( j < matrix_.columns(), "Invalid column access index" );
      return matrix_(i,j) * scalar_;
   }
   //**********************************************************************************************

   //**At function*********************************************************************************
   /*!\brief Checked access to the matrix elements.
   //
   // \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
   // \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
   // \return The resulting value.
   // \exception std::out_of_range Invalid matrix access index.
   */
   inline ReturnType at( size_t i, size_t j ) const {
      if( i >= matrix_.rows() ) {
         BLAZE_THROW_OUT_OF_RANGE( "Invalid row access index" );
      }
      if( j >= matrix_.columns() ) {
         BLAZE_THROW_OUT_OF_RANGE( "Invalid column access index" );
      }
      return (*this)(i,j);
   }
   //**********************************************************************************************

   //**Rows function*******************************************************************************
   /*!\brief Returns the current number of rows of the matrix.
   //
   // \return The number of rows of the matrix.
   */
   inline size_t rows() const {
      return matrix_.rows();
   }
   //**********************************************************************************************

   //**Columns function****************************************************************************
   /*!\brief Returns the current number of columns of the matrix.
   //
   // \return The number of columns of the matrix.
   */
   inline size_t columns() const {
      return matrix_.columns();
   }
   //**********************************************************************************************

   //**Left operand access*************************************************************************
   /*!\brief Returns the left-hand side dense matrix operand.
   //
   // \return The left-hand side dense matrix operand.
   */
   inline LeftOperand leftOperand() const {
      return matrix_;
   }
   //**********************************************************************************************

   //**Right operand access************************************************************************
   /*!\brief Returns the right-hand side scalar operand.
   //
   // \return The right-hand side scalar operand.
   */
   inline RightOperand rightOperand() const {
      return scalar_;
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression can alias with the given address \a alias.
   //
   // \param alias The alias to be checked.
   // \return \a true in case the expression can alias, \a false otherwise.
   */
   template< typename T >
   inline bool canAlias( const T* alias ) const {
      return matrix_.canAlias( alias );
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression is aliased with the given address \a alias.
   //
   // \param alias The alias to be checked.
   // \return \a true in case an alias effect is detected, \a false otherwise.
   */
   template< typename T >
   inline bool isAliased( const T* alias ) const {
      return matrix_.isAliased( alias );
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the operands of the expression are properly aligned in memory.
   //
   // \return \a true in case the operands are aligned, \a false if not.
   */
   inline bool isAligned() const {
      return matrix_.isAligned();
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression can be used in SMP assignments.
   //
   // \return \a true in case the expression can be used in SMP assignments, \a false if not.
   */
   inline bool canSMPAssign() const noexcept {
      return ( !BLAZE_BLAS_MODE ||
               !BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION ||
               !BLAZE_BLAS_IS_PARALLEL ||
               ( rows() * columns() < TDMATTDMATMULT_THRESHOLD ) ) &&
             ( rows() * columns() >= SMP_TDMATTDMATMULT_THRESHOLD );
   }
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   LeftOperand  matrix_;  //!< Left-hand side dense matrix of the multiplication expression.
   RightOperand scalar_;  //!< Right-hand side scalar of the multiplication expression.
   //**********************************************************************************************

   //**Assignment to dense matrices****************************************************************
   /*!\brief Assignment of a scaled transpose dense matrix-transpose dense matrix multiplication
   //        to a dense matrix (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side scaled multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a scaled transpose dense
   // matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      assign( DenseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL ) {
         return;
      }
      else if( left.columns() == 0UL ) {
         reset( ~lhs );
         return;
      }

      LT A( serial( left  ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( right ) );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == left.rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == left.columns()  , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == right.rows()    , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == right.columns() , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns(), "Invalid number of columns" );

      DMatScalarMultExpr::selectAssignKernel( ~lhs, A, B, rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Assignment to dense matrices (kernel selection)*********************************************
   /*!\brief Selection of the kernel for an assignment of a scaled transpose dense matrix-
   //        transpose dense matrix multiplication to a dense matrix (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline void selectAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      if( ( IsDiagonal<MT4>::value ) ||
          ( !BLAZE_DEBUG_MODE && A.rows() <= SIMDSIZE*10UL ) ||
          ( C.rows() * C.columns() < TDMATTDMATMULT_THRESHOLD ) )
         selectSmallAssignKernel( C, A, B, scalar );
      else
         selectBlasAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Default assignment to dense matrices (general/general)**************************************
   /*!\brief Default assignment of a scaled general transpose dense matrix-general transpose
   //        dense matrix multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default assignment of a scaled general transpose dense matrix-
   // general transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, Not< IsDiagonal<MT5> > > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( SYM || HERM || LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t kbegin( ( IsLower<MT5>::value )
                              ?( IsStrictlyLower<MT5>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t kend( ( IsUpper<MT5>::value )
                            ?( IsStrictlyUpper<MT5>::value ? j : j+1UL )
                            :( K ) );
         BLAZE_INTERNAL_ASSERT( kbegin <= kend, "Invalid loop indices detected" );

         if( IsStrictlyTriangular<MT5>::value && kbegin == kend ) {
            for( size_t i=0UL; i<M; ++i ) {
               reset( C(i,j) );
            }
            continue;
         }

         {
            const size_t ibegin( ( IsLower<MT4>::value )
                                 ?( ( IsStrictlyLower<MT4>::value )
                                    ?( LOW ? max(j,kbegin+1UL) : kbegin+1UL )
                                    :( LOW ? max(j,kbegin) : kbegin ) )
                                 :( LOW ? j : 0UL ) );
            const size_t iend( ( IsUpper<MT4>::value )
                               ?( ( IsStrictlyUpper<MT4>::value )
                                  ?( UPP ? min(j+1UL,kbegin) : kbegin )
                                  :( UPP ? min(j,kbegin)+1UL : kbegin+1UL ) )
                               :( UPP ? j+1UL : M ) );

            if( ( IsLower<MT4>::value && IsLower<MT5>::value ) || LOW ) {
               for( size_t i=0UL; i<ibegin; ++i ) {
                  reset( C(i,j) );
               }
            }
            else if( IsStrictlyLower<MT4>::value ) {
               reset( C(0UL,j) );
            }
            for( size_t i=ibegin; i<iend; ++i ) {
               C(i,j) = A(i,kbegin) * B(kbegin,j);
            }
            if( ( IsUpper<MT4>::value && IsUpper<MT5>::value ) || UPP ) {
               for( size_t i=iend; i<M; ++i ) {
                  reset( C(i,j) );
               }
            }
            else if( IsStrictlyUpper<MT4>::value ) {
               reset( C(M-1UL,j) );
            }
         }

         for( size_t k=kbegin+1UL; k<kend; ++k )
         {
            const size_t ibegin( ( IsLower<MT4>::value )
                                 ?( ( IsStrictlyLower<MT4>::value )
                                    ?( SYM || HERM || LOW ? max( j, k+1UL ) : k+1UL )
                                    :( SYM || HERM || LOW ? max( j, k ) : k ) )
                                 :( SYM || HERM || LOW ? j : 0UL ) );
            const size_t iend( ( IsUpper<MT4>::value )
                               ?( ( IsStrictlyUpper<MT4>::value )
                                  ?( UPP ? min(j+1UL,k-1UL) : k-1UL )
                                  :( UPP ? min(j+1UL,k) : k ) )
                               :( UPP ? j+1UL : M ) );

            if( ( SYM || HERM || LOW || UPP ) && ( ibegin > iend ) ) continue;
            BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

            for( size_t i=ibegin; i<iend; ++i ) {
               C(i,j) += A(i,k) * B(k,j);
            }
            if( IsUpper<MT4>::value ) {
               C(iend,j) = A(iend,k) * B(k,j);
            }
         }

         {
            const size_t ibegin( ( IsLower<MT4>::value && IsLower<MT5>::value )
                                 ?( IsStrictlyLower<MT4>::value || IsStrictlyLower<MT5>::value ? j+1UL : j )
                                 :( ( SYM || HERM || LOW )?( j ):( 0UL ) ) );
            const size_t iend( ( IsUpper<MT4>::value && IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT4>::value || IsStrictlyUpper<MT5>::value ? j : j+1UL )
                               :( UPP ? j+1UL : M ) );

            if( ( SYM || HERM || LOW || UPP ) && ( ibegin > iend ) ) continue;
            BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

            for( size_t i=ibegin; i<iend; ++i ) {
               C(i,j) *= scalar;
            }
         }
      }

      if( SYM || HERM ) {
         for( size_t j=1UL; j<N; ++j ) {
            for( size_t i=0UL; i<j; ++i ) {
               C(i,j) = HERM ? conj( C(j,i) ) : C(j,i);
            }
         }
      }
   }
   //**********************************************************************************************

   //**Default assignment to dense matrices (general/diagonal)*************************************
   /*!\brief Default assignment of a scaled general transpose dense matrix-diagonal transpose
   //        dense matrix multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default assignment of a scaled general transpose dense matrix-
   // diagonal transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, IsDiagonal<MT5> > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t ibegin( ( IsLower<MT4>::value )
                              ?( IsStrictlyLower<MT4>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t iend( ( IsUpper<MT4>::value )
                            ?( IsStrictlyUpper<MT4>::value ? j : j+1UL )
                            :( M ) );
         BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

         if( IsLower<MT4>::value ) {
            for( size_t i=0UL; i<ibegin; ++i ) {
               reset( C(i,j) );
            }
         }
         for( size_t i=ibegin; i<iend; ++i ) {
            C(i,j) = A(i,j) * B(j,j) * scalar;
         }
         if( IsUpper<MT4>::value ) {
            for( size_t i=iend; i<M; ++i ) {
               reset( C(i,j) );
            }
         }
      }
   }
   //**********************************************************************************************

   //**Default assignment to dense matrices (diagonal/general)*************************************
   /*!\brief Default assignment of a scaled diagonal transpose dense matrix-general transpose
   //        dense matrix multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default assignment of a scaled diagonal transpose dense matrix-
   // general transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< IsDiagonal<MT4>, Not< IsDiagonal<MT5> > > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t ibegin( ( IsLower<MT5>::value )
                              ?( IsStrictlyLower<MT5>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t iend( ( IsUpper<MT5>::value )
                            ?( IsStrictlyUpper<MT5>::value ? j : j+1UL )
                            :( M ) );
         BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

         if( IsLower<MT4>::value ) {
            for( size_t i=0UL; i<ibegin; ++i ) {
               reset( C(i,j) );
            }
         }
         for( size_t i=ibegin; i<iend; ++i ) {
            C(i,j) = A(i,i) * B(i,j) * scalar;
         }
         if( IsUpper<MT4>::value ) {
            for( size_t i=iend; i<M; ++i ) {
               reset( C(i,j) );
            }
         }
      }
   }
   //**********************************************************************************************

   //**Default assignment to dense matrices (diagonal/diagonal)************************************
   /*!\brief Default assignment of a scaled diagonal transpose dense matrix-diagonal transpose
   //        dense matrix multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default assignment of a scaled diagonal transpose dense matrix-
   // diagonal transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< IsDiagonal<MT4>, IsDiagonal<MT5> > >
      selectDefaultAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      reset( C );

      for( size_t i=0UL; i<A.rows(); ++i ) {
         C(i,i) = A(i,i) * B(i,i) * scalar;
      }
   }
   //**********************************************************************************************

   //**Default assignment to dense matrices (small matrices)***************************************
   /*!\brief Default assignment of a small scaled transpose dense matrix-transpose dense matrix
   //        multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the assignment of a scaled transpose
   // dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectDefaultAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Vectorized default assignment to row-major dense matrices (small matrices)******************
   /*!\brief Vectorized default assignment of a small scaled transpose dense matrix-transpose
   //        dense matrix multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default assignment of a scaled transpose dense
   // matrix-transpose dense matrix multiplication expression to a row-major dense matrix.
   // This kernel is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallAssignKernel( DenseMatrix<MT3,false>& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT4 );
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT5 );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType_<MT4> );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType_<MT5> );

      const ForwardFunctor fwd;

      if( IsResizable<MT4>::value && !IsResizable<MT5>::value ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         assign( ~C, fwd( A * tmp ) * scalar );
      }
      else if( !IsResizable<MT4>::value && IsResizable<MT5>::value ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         assign( ~C, fwd( tmp * B ) * scalar );
      }
      else if( B.rows() * B.columns() <= A.rows() * A.columns() ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         assign( ~C, fwd( A * tmp ) * scalar );
      }
      else {
         const OppositeType_<MT4> tmp( serial( A ) );
         assign( ~C, fwd( tmp * B ) * scalar );
      }
   }
   //**********************************************************************************************

   //**Vectorized default assignment to column-major dense matrices (small matrices)***************
   /*!\brief Vectorized default assignment of a small scaled transpose dense matrix-transpose
   //        dense matrix multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default assignment of a scaled transpose dense
   // matrix-transpose dense matrix multiplication expression to a column-major dense matrix.
   // This kernel is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallAssignKernel( DenseMatrix<MT3,true>& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT4>::value );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( SYM || HERM || LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      const size_t ipos( remainder ? ( M & size_t(-SIMDSIZE) ) : M );
      BLAZE_INTERNAL_ASSERT( !remainder || ( M - ( M % SIMDSIZE ) ) == ipos, "Invalid end calculation" );

      const SIMDType factor( set( scalar ) );

      if( LOW && UPP && M > SIMDSIZE*3UL ) {
         reset( ~C );
      }

      {
         size_t i( 0UL );

         if( IsIntegral<ElementType>::value )
         {
            for( ; !SYM && !HERM && !LOW && !UPP && (i+SIMDSIZE*7UL) < ipos; i+=SIMDSIZE*8UL ) {
               for( size_t j=0UL; j<N; ++j )
               {
                  const size_t kbegin( ( IsLower<MT5>::value )
                                       ?( ( IsUpper<MT4>::value )
                                          ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                          :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                       :( IsUpper<MT4>::value ? i : 0UL ) );
                  const size_t kend( ( IsUpper<MT5>::value )
                                     ?( ( IsLower<MT4>::value )
                                        ?( min( i+SIMDSIZE*8UL, K, ( IsStrictlyUpper<MT5>::value ? j : j+1UL ) ) )
                                        :( IsStrictlyUpper<MT5>::value ? j : j+1UL ) )
                                     :( IsLower<MT4>::value ? min( i+SIMDSIZE*8UL, K ) : K ) );

                  SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

                  for( size_t k=kbegin; k<kend; ++k ) {
                     const SIMDType b1( set( B(k,j) ) );
                     xmm1 += A.load(i             ,k) * b1;
                     xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
                     xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
                     xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
                     xmm5 += A.load(i+SIMDSIZE*4UL,k) * b1;
                     xmm6 += A.load(i+SIMDSIZE*5UL,k) * b1;
                     xmm7 += A.load(i+SIMDSIZE*6UL,k) * b1;
                     xmm8 += A.load(i+SIMDSIZE*7UL,k) * b1;
                  }

                  (~C).store( i             , j, xmm1 * factor );
                  (~C).store( i+SIMDSIZE    , j, xmm2 * factor );
                  (~C).store( i+SIMDSIZE*2UL, j, xmm3 * factor );
                  (~C).store( i+SIMDSIZE*3UL, j, xmm4 * factor );
                  (~C).store( i+SIMDSIZE*4UL, j, xmm5 * factor );
                  (~C).store( i+SIMDSIZE*5UL, j, xmm6 * factor );
                  (~C).store( i+SIMDSIZE*6UL, j, xmm7 * factor );
                  (~C).store( i+SIMDSIZE*7UL, j, xmm8 * factor );
               }
            }
         }

         for( ; !SYM && !HERM && !LOW && !UPP && (i+SIMDSIZE*4UL) < ipos; i+=SIMDSIZE*5UL )
         {
            size_t j( 0UL );

            for( ; (j+2UL) <= N; j+=2UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*5UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*5UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( A.load(i             ,k) );
                  const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
                  const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
                  const SIMDType a4( A.load(i+SIMDSIZE*3UL,k) );
                  const SIMDType a5( A.load(i+SIMDSIZE*4UL,k) );
                  const SIMDType b1( set( B(k,j    ) ) );
                  const SIMDType b2( set( B(k,j+1UL) ) );
                  xmm1  += a1 * b1;
                  xmm2  += a2 * b1;
                  xmm3  += a3 * b1;
                  xmm4  += a4 * b1;
                  xmm5  += a5 * b1;
                  xmm6  += a1 * b2;
                  xmm7  += a2 * b2;
                  xmm8  += a3 * b2;
                  xmm9  += a4 * b2;
                  xmm10 += a5 * b2;
               }

               (~C).store( i             , j    , xmm1  * factor );
               (~C).store( i+SIMDSIZE    , j    , xmm2  * factor );
               (~C).store( i+SIMDSIZE*2UL, j    , xmm3  * factor );
               (~C).store( i+SIMDSIZE*3UL, j    , xmm4  * factor );
               (~C).store( i+SIMDSIZE*4UL, j    , xmm5  * factor );
               (~C).store( i             , j+1UL, xmm6  * factor );
               (~C).store( i+SIMDSIZE    , j+1UL, xmm7  * factor );
               (~C).store( i+SIMDSIZE*2UL, j+1UL, xmm8  * factor );
               (~C).store( i+SIMDSIZE*3UL, j+1UL, xmm9  * factor );
               (~C).store( i+SIMDSIZE*4UL, j+1UL, xmm10 * factor );
            }

            if( j < N )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*5UL, K ) ):( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType b1( set( B(k,j) ) );
                  xmm1 += A.load(i             ,k) * b1;
                  xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
                  xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
                  xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
                  xmm5 += A.load(i+SIMDSIZE*4UL,k) * b1;
               }

               (~C).store( i             , j, xmm1 * factor );
               (~C).store( i+SIMDSIZE    , j, xmm2 * factor );
               (~C).store( i+SIMDSIZE*2UL, j, xmm3 * factor );
               (~C).store( i+SIMDSIZE*3UL, j, xmm4 * factor );
               (~C).store( i+SIMDSIZE*4UL, j, xmm5 * factor );
            }
         }

         for( ; !( LOW && UPP ) && (i+SIMDSIZE*3UL) < ipos; i+=SIMDSIZE*4UL )
         {
            const size_t jend( SYM || HERM || LOW ? min(i+SIMDSIZE*4UL,N) : N );
            size_t j( UPP ? i : 0UL );

            for( ; (j+2UL) <= jend; j+=2UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*4UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*4UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( A.load(i             ,k) );
                  const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
                  const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
                  const SIMDType a4( A.load(i+SIMDSIZE*3UL,k) );
                  const SIMDType b1( set( B(k,j    ) ) );
                  const SIMDType b2( set( B(k,j+1UL) ) );
                  xmm1 += a1 * b1;
                  xmm2 += a2 * b1;
                  xmm3 += a3 * b1;
                  xmm4 += a4 * b1;
                  xmm5 += a1 * b2;
                  xmm6 += a2 * b2;
                  xmm7 += a3 * b2;
                  xmm8 += a4 * b2;
               }

               (~C).store( i             , j    , xmm1 * factor );
               (~C).store( i+SIMDSIZE    , j    , xmm2 * factor );
               (~C).store( i+SIMDSIZE*2UL, j    , xmm3 * factor );
               (~C).store( i+SIMDSIZE*3UL, j    , xmm4 * factor );
               (~C).store( i             , j+1UL, xmm5 * factor );
               (~C).store( i+SIMDSIZE    , j+1UL, xmm6 * factor );
               (~C).store( i+SIMDSIZE*2UL, j+1UL, xmm7 * factor );
               (~C).store( i+SIMDSIZE*3UL, j+1UL, xmm8 * factor );
            }

            if( j < jend )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*4UL, K ) ):( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType b1( set( B(k,j) ) );
                  xmm1 += A.load(i             ,k) * b1;
                  xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
                  xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
                  xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
               }

               (~C).store( i             , j, xmm1 * factor );
               (~C).store( i+SIMDSIZE    , j, xmm2 * factor );
               (~C).store( i+SIMDSIZE*2UL, j, xmm3 * factor );
               (~C).store( i+SIMDSIZE*3UL, j, xmm4 * factor );
            }
         }

         for( ; (i+SIMDSIZE*2UL) < ipos; i+=SIMDSIZE*3UL )
         {
            const size_t jend( SYM || HERM || LOW ? min(i+SIMDSIZE*3UL,N) : N );
            size_t j( UPP ? i : 0UL );

            for( ; (j+2UL) <= jend; j+=2UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*3UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*3UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( A.load(i             ,k) );
                  const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
                  const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
                  const SIMDType b1( set( B(k,j    ) ) );
                  const SIMDType b2( set( B(k,j+1UL) ) );
                  xmm1 += a1 * b1;
                  xmm2 += a2 * b1;
                  xmm3 += a3 * b1;
                  xmm4 += a1 * b2;
                  xmm5 += a2 * b2;
                  xmm6 += a3 * b2;
               }

               (~C).store( i             , j    , xmm1 * factor );
               (~C).store( i+SIMDSIZE    , j    , xmm2 * factor );
               (~C).store( i+SIMDSIZE*2UL, j    , xmm3 * factor );
               (~C).store( i             , j+1UL, xmm4 * factor );
               (~C).store( i+SIMDSIZE    , j+1UL, xmm5 * factor );
               (~C).store( i+SIMDSIZE*2UL, j+1UL, xmm6 * factor );
            }

            if( j < jend )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*3UL, K ) ):( K ) );

               SIMDType xmm1, xmm2, xmm3;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType b1( set( B(k,j) ) );
                  xmm1 += A.load(i             ,k) * b1;
                  xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
                  xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
               }

               (~C).store( i             , j, xmm1 * factor );
               (~C).store( i+SIMDSIZE    , j, xmm2 * factor );
               (~C).store( i+SIMDSIZE*2UL, j, xmm3 * factor );
            }
         }

         for( ; (i+SIMDSIZE) < ipos; i+=SIMDSIZE*2UL )
         {
            const size_t jend( SYM || HERM || LOW ? min(i+SIMDSIZE*2UL,N) : N );
            size_t j( UPP ? i : 0UL );

            for( ; (j+4UL) <= jend; j+=4UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( A.load(i         ,k) );
                  const SIMDType a2( A.load(i+SIMDSIZE,k) );
                  const SIMDType b1( set( B(k,j    ) ) );
                  const SIMDType b2( set( B(k,j+1UL) ) );
                  const SIMDType b3( set( B(k,j+2UL) ) );
                  const SIMDType b4( set( B(k,j+3UL) ) );
                  xmm1 += a1 * b1;
                  xmm2 += a2 * b1;
                  xmm3 += a1 * b2;
                  xmm4 += a2 * b2;
                  xmm5 += a1 * b3;
                  xmm6 += a2 * b3;
                  xmm7 += a1 * b4;
                  xmm8 += a2 * b4;
               }

               (~C).store( i         , j    , xmm1 * factor );
               (~C).store( i+SIMDSIZE, j    , xmm2 * factor );
               (~C).store( i         , j+1UL, xmm3 * factor );
               (~C).store( i+SIMDSIZE, j+1UL, xmm4 * factor );
               (~C).store( i         , j+2UL, xmm5 * factor );
               (~C).store( i+SIMDSIZE, j+2UL, xmm6 * factor );
               (~C).store( i         , j+3UL, xmm7 * factor );
               (~C).store( i+SIMDSIZE, j+3UL, xmm8 * factor );
            }

            for( ; (j+3UL) <= jend; j+=3UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType a1( A.load(i         ,k) );
                  const SIMDType a2( A.load(i+SIMDSIZE,k) );
                  const SIMDType b1( set( B(k,j    ) ) );
                  const SIMDType b2( set( B(k,j+1UL) ) );
                  const SIMDType b3( set( B(k,j+2UL) ) );
                  xmm1 += a1 * b1;
                  xmm2 += a2 * b1;
                  xmm3 += a1 * b2;
                  xmm4 += a2 * b2;
                  xmm5 += a1 * b3;
                  xmm6 += a2 * b3;
               }

               (~C).store( i         , j    , xmm1 * factor );
               (~C).store( i+SIMDSIZE, j    , xmm2 * factor );
               (~C).store( i         , j+1UL, xmm3 * factor );
               (~C).store( i+SIMDSIZE, j+1UL, xmm4 * factor );
               (~C).store( i         , j+2UL, xmm5 * factor );
               (~C).store( i+SIMDSIZE, j+2UL, xmm6 * factor );
            }

            for( ; (j+2UL) <= jend; j+=2UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;
               size_t k( kbegin );

               for( ; (k+2UL) <= kend; k+=2UL ) {
                  const SIMDType a1( A.load(i         ,k    ) );
                  const SIMDType a2( A.load(i+SIMDSIZE,k    ) );
                  const SIMDType a3( A.load(i         ,k+1UL) );
                  const SIMDType a4( A.load(i+SIMDSIZE,k+1UL) );
                  const SIMDType b1( set( B(k    ,j    ) ) );
                  const SIMDType b2( set( B(k    ,j+1UL) ) );
                  const SIMDType b3( set( B(k+1UL,j    ) ) );
                  const SIMDType b4( set( B(k+1UL,j+1UL) ) );
                  xmm1 += a1 * b1;
                  xmm2 += a2 * b1;
                  xmm3 += a1 * b2;
                  xmm4 += a2 * b2;
                  xmm5 += a3 * b3;
                  xmm6 += a4 * b3;
                  xmm7 += a3 * b4;
                  xmm8 += a4 * b4;
               }

               for( ; k<kend; ++k ) {
                  const SIMDType a1( A.load(i         ,k) );
                  const SIMDType a2( A.load(i+SIMDSIZE,k) );
                  const SIMDType b1( set( B(k,j    ) ) );
                  const SIMDType b2( set( B(k,j+1UL) ) );
                  xmm1 += a1 * b1;
                  xmm2 += a2 * b1;
                  xmm3 += a1 * b2;
                  xmm4 += a2 * b2;
               }

               (~C).store( i         , j    , (xmm1+xmm5) * factor );
               (~C).store( i+SIMDSIZE, j    , (xmm2+xmm6) * factor );
               (~C).store( i         , j+1UL, (xmm3+xmm7) * factor );
               (~C).store( i+SIMDSIZE, j+1UL, (xmm4+xmm8) * factor );
            }

            if( j < jend )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*2UL, K ) ):( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4;
               size_t k( kbegin );

               for( ; (k+2UL) <= kend; k+=2UL ) {
                  const SIMDType b1( set( B(k    ,j) ) );
                  const SIMDType b2( set( B(k+1UL,j) ) );
                  xmm1 += A.load(i         ,k    ) * b1;
                  xmm2 += A.load(i+SIMDSIZE,k    ) * b1;
                  xmm3 += A.load(i         ,k+1UL) * b2;
                  xmm4 += A.load(i+SIMDSIZE,k+1UL) * b2;
               }

               for( ; k<kend; ++k ) {
                  const SIMDType b1( set( B(k,j) ) );
                  xmm1 += A.load(i         ,k) * b1;
                  xmm2 += A.load(i+SIMDSIZE,k) * b1;
               }

               (~C).store( i         , j, (xmm1+xmm3) * factor );
               (~C).store( i+SIMDSIZE, j, (xmm2+xmm4) * factor );
            }
         }

         for( ; i<ipos; i+=SIMDSIZE )
         {
            const size_t jend( SYM || HERM || LOW ? min(i+SIMDSIZE,N) : N );
            size_t j( UPP ? i : 0UL );

            for( ; (j+4UL) <= jend; j+=4UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL )
                                  :( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;
               size_t k( kbegin );

               for( ; (k+2UL) <= kend; k+=2UL ) {
                  const SIMDType a1( A.load(i,k    ) );
                  const SIMDType a2( A.load(i,k+1UL) );
                  xmm1 += a1 * set( B(k    ,j    ) );
                  xmm2 += a1 * set( B(k    ,j+1UL) );
                  xmm3 += a1 * set( B(k    ,j+2UL) );
                  xmm4 += a1 * set( B(k    ,j+3UL) );
                  xmm5 += a2 * set( B(k+1UL,j    ) );
                  xmm6 += a2 * set( B(k+1UL,j+1UL) );
                  xmm7 += a2 * set( B(k+1UL,j+2UL) );
                  xmm8 += a2 * set( B(k+1UL,j+3UL) );
               }

               for( ; k<kend; ++k ) {
                  const SIMDType a1( A.load(i,k) );
                  xmm1 += a1 * set( B(k,j    ) );
                  xmm2 += a1 * set( B(k,j+1UL) );
                  xmm3 += a1 * set( B(k,j+2UL) );
                  xmm4 += a1 * set( B(k,j+3UL) );
               }

               (~C).store( i, j    , (xmm1+xmm5) * factor );
               (~C).store( i, j+1UL, (xmm2+xmm6) * factor );
               (~C).store( i, j+2UL, (xmm3+xmm7) * factor );
               (~C).store( i, j+3UL, (xmm4+xmm8) * factor );
            }

            for( ; (j+3UL) <= jend; j+=3UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL )
                                  :( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;
               size_t k( kbegin );

               for( ; (k+2UL) <= kend; k+=2UL ) {
                  const SIMDType a1( A.load(i,k    ) );
                  const SIMDType a2( A.load(i,k+1UL) );
                  xmm1 += a1 * set( B(k    ,j    ) );
                  xmm2 += a1 * set( B(k    ,j+1UL) );
                  xmm3 += a1 * set( B(k    ,j+2UL) );
                  xmm4 += a2 * set( B(k+1UL,j    ) );
                  xmm5 += a2 * set( B(k+1UL,j+1UL) );
                  xmm6 += a2 * set( B(k+1UL,j+2UL) );
               }

               for( ; k<kend; ++k ) {
                  const SIMDType a1( A.load(i,k) );
                  xmm1 += a1 * set( B(k,j    ) );
                  xmm2 += a1 * set( B(k,j+1UL) );
                  xmm3 += a1 * set( B(k,j+2UL) );
               }

               (~C).store( i, j    , (xmm1+xmm4) * factor );
               (~C).store( i, j+1UL, (xmm2+xmm5) * factor );
               (~C).store( i, j+2UL, (xmm3+xmm6) * factor );
            }

            for( ; (j+2UL) <= jend; j+=2UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL )
                                  :( K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4;
               size_t k( kbegin );

               for( ; k<kend; ++k ) {
                  const SIMDType a1( A.load(i,k) );
                  xmm1 += a1 * set( B(k,j    ) );
                  xmm2 += a1 * set( B(k,j+1UL) );
               }

               for( ; (k+2UL) <= kend; k+=2UL ) {
                  const SIMDType a1( A.load(i,k    ) );
                  const SIMDType a2( A.load(i,k+1UL) );
                  xmm1 += a1 * set( B(k    ,j    ) );
                  xmm2 += a1 * set( B(k    ,j+1UL) );
                  xmm3 += a2 * set( B(k+1UL,j    ) );
                  xmm4 += a2 * set( B(k+1UL,j+1UL) );
               }

               (~C).store( i, j    , (xmm1+xmm3) * factor );
               (~C).store( i, j+1UL, (xmm2+xmm4) * factor );
            }

            if( j < jend )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );

               SIMDType xmm1, xmm2;
               size_t k( kbegin );

               for( ; (k+2UL) <= K; k+=2UL ) {
                  xmm1 += A.load(i,k    ) * set( B(k    ,j) );
                  xmm2 += A.load(i,k+1UL) * set( B(k+1UL,j) );
               }

               for( ; k<K; ++k ) {
                  xmm1 += A.load(i,k) * set( B(k,j) );
               }

               (~C).store( i, j, (xmm1+xmm2) * factor );
            }
         }

         for( ; remainder && i<M; ++i )
         {
            size_t j( LOW && UPP ? i : 0UL );

            for( ; (j+2UL) <= N; j+=2UL )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL )
                                  :( K ) );

               ElementType value1{};
               ElementType value2{};

               for( size_t k=kbegin; k<kend; ++k ) {
                  value1 += A(i,k) * B(k,j    );
                  value2 += A(i,k) * B(k,j+1UL);
               }

               (~C)(i,j    ) = value1 * scalar;
               (~C)(i,j+1UL) = value2 * scalar;
            }

            if( j < N )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );

               ElementType value{};

               for( size_t k=kbegin; k<K; ++k ) {
                  value += A(i,k) * B(k,j);
               }

               (~C)(i,j) = value * scalar;
            }
         }
      }

      if( ( SYM || HERM ) && ( M > SIMDSIZE*4UL ) ) {
         for( size_t j=SIMDSIZE*4UL; j<N; ++j ) {
            const size_t iend( ( SIMDSIZE*4UL ) * ( j / (SIMDSIZE*4UL) ) );
            for( size_t i=0UL; i<iend; ++i ) {
               (~C)(i,j) = HERM ? conj( (~C)(j,i) ) : (~C)(j,i);
            }
         }
      }
      else if( LOW && !UPP && M > SIMDSIZE*4UL ) {
         for( size_t j=SIMDSIZE*4UL; j<N; ++j ) {
            const size_t iend( ( SIMDSIZE*4UL ) * ( j / (SIMDSIZE*4UL) ) );
            for( size_t i=0UL; i<iend; ++i ) {
               reset( (~C)(i,j) );
            }
         }
      }
      else if( !LOW && UPP && M > SIMDSIZE*4UL ) {
         for( size_t i=SIMDSIZE*4UL; i<M; ++i ) {
            const size_t jend( ( SIMDSIZE*4UL ) * ( i / (SIMDSIZE*4UL) ) );
            for( size_t j=0UL; j<jend; ++j ) {
               reset( (~C)(i,j) );
            }
         }
      }
   }
   //**********************************************************************************************

   //**Default assignment to dense matrices (large matrices)***************************************
   /*!\brief Default assignment of a large scaled transpose dense matrix-transpose dense matrix
   //        multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the assignment of a scaled transpose
   // dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectLargeAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectDefaultAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Vectorized default assignment to dense matrices (large matrices)****************************
   /*!\brief Vectorized default assignment of a large scaled transpose dense matrix-transpose
   //        dense matrix multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default assignment of a scaled transpose dense
   // matrix-transpose dense matrix multiplication expression to a dense matrix. This kernel
   // is optimized for large matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectLargeAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      if( SYM )
         smmm( C, A, B, scalar );
      else if( HERM )
         hmmm( C, A, B, scalar );
      else if( LOW )
         lmmm( C, A, B, scalar, ST2(0) );
      else if( UPP )
         ummm( C, A, B, scalar, ST2(0) );
      else
         mmm( C, A, B, scalar, ST2(0) );
   }
   //**********************************************************************************************

   //**BLAS-based assignment to dense matrices (default)*******************************************
   /*!\brief Default assignment of a scaled transpose dense matrix-transpose dense matrix
   //        multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the assignment of a large scaled
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseBlasKernel<MT3,MT4,MT5,ST2> >
      selectBlasAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectLargeAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**BLAS-based assignment to dense matrices*****************************************************
#if BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION
   /*!\brief BLAS-based assignment of a scaled transpose dense matrix-transpose dense matrix
   //        multiplication (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function performs the scaled transpose dense matrix-transpose dense matrix multiplication
   // based on the according BLAS functionality.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseBlasKernel<MT3,MT4,MT5,ST2> >
      selectBlasAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      using ET = ElementType_<MT3>;

      if( IsTriangular<MT4>::value ) {
         assign( C, B );
         trmm( C, A, CblasLeft, ( IsLower<MT4>::value )?( CblasLower ):( CblasUpper ), ET(scalar) );
      }
      else if( IsTriangular<MT5>::value ) {
         assign( C, A );
         trmm( C, B, CblasRight, ( IsLower<MT5>::value )?( CblasLower ):( CblasUpper ), ET(scalar) );
      }
      else {
         gemm( C, A, B, ET(scalar), ET(0) );
      }
   }
#endif
   //**********************************************************************************************

   //**Assignment to sparse matrices***************************************************************
   /*!\brief Assignment of a scaled transpose dense matrix-transpose dense matrix multiplication
   //        to a sparse matrix (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side sparse matrix.
   // \param rhs The right-hand side scaled multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a scaled transpose dense
   // matrix-transpose dense matrix multiplication expression to a sparse matrix.
   */
   template< typename MT  // Type of the target sparse matrix
           , bool SO >    // Storage order of the target sparse matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      assign( SparseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      using TmpType = IfTrue_< SO, ResultType, OppositeType >;

      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MATRICES_MUST_HAVE_SAME_STORAGE_ORDER( MT, TmpType );
      BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( TmpType );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      const TmpType tmp( serial( rhs ) );
      assign( ~lhs, fwd( tmp ) );
   }
   //**********************************************************************************************

   //**Restructuring assignment to row-major matrices**********************************************
   /*!\brief Restructuring assignment of a scaled transpose dense matrix-transpose dense matrix
   //        multiplication to a row-major matrix (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side scaled multiplication expression to be assigned.
   // \return void
   //
   // This function implements the symmetry-based restructuring assignment of a scaled transpose
   // dense matrix-transpose dense matrix multiplication expression to a row-major matrix. Due to
   // the explicit application of the SFINAE principle this function can only be selected by the
   // compiler in case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      assign( Matrix<MT,false>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         assign( ~lhs, fwd( trans( left ) * trans( right ) ) * rhs.scalar_ );
      else if( IsSymmetric<MT1>::value )
         assign( ~lhs, fwd( trans( left ) * right ) * rhs.scalar_ );
      else
         assign( ~lhs, fwd( left * trans( right ) ) * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Addition assignment to dense matrices*******************************************************
   /*!\brief Addition assignment of a scaled transpose dense matrix-transpose dense matrix
   //        multiplication to a dense matrix (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be added.
   // \return void
   //
   // This function implements the performance optimized addition assignment of a scaled transpose
   // dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      addAssign( DenseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || left.columns() == 0UL ) {
         return;
      }

      LT A( serial( left  ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( right ) );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == left.rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == left.columns()  , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == right.rows()    , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == right.columns() , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns(), "Invalid number of columns" );

      DMatScalarMultExpr::selectAddAssignKernel( ~lhs, A, B, rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Addition assignment to dense matrices (kernel selection)************************************
   /*!\brief Selection of the kernel for an addition assignment of a scaled transpose dense
   //        matrix-transpose dense matrix multiplication to a dense matrix (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline void selectAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      if( ( IsDiagonal<MT4>::value ) ||
          ( !BLAZE_DEBUG_MODE && A.rows() <= SIMDSIZE*10UL ) ||
          ( C.rows() * C.columns() < TDMATTDMATMULT_THRESHOLD ) )
         selectSmallAddAssignKernel( C, A, B, scalar );
      else
         selectBlasAddAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (general/general)*****************************
   /*!\brief Default addition assignment of a scaled general transpose dense matrix-general
   //        transpose dense matrix multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default addition assignment of a scaled general transpose
   // dense matrix-general transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, Not< IsDiagonal<MT5> > > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      const ResultType tmp( serial( A * B * scalar ) );
      addAssign( C, tmp );
   }
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (general/diagonal)****************************
   /*!\brief Default addition assignment of a scaled general transpose dense matrix-diagonal
   //        transpose dense matrix multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default addition assignment of a scaled general transpose
   // dense matrix-diagonal transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, IsDiagonal<MT5> > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t ibegin( ( IsLower<MT4>::value )
                              ?( IsStrictlyLower<MT4>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t iend( ( IsUpper<MT4>::value )
                            ?( IsStrictlyUpper<MT4>::value ? j : j+1UL )
                            :( M ) );
         BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

         const size_t inum( iend - ibegin );
         const size_t ipos( ibegin + ( inum & size_t(-2) ) );

         for( size_t i=ibegin; i<ipos; i+=2UL ) {
            C(i    ,j) += A(i    ,j) * B(j,j) * scalar;
            C(i+1UL,j) += A(i+1UL,j) * B(j,j) * scalar;
         }
         if( ipos < iend ) {
            C(ipos,j) += A(ipos,j) * B(j,j) * scalar;
         }
      }
   }
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (diagonal/general)****************************
   /*!\brief Default addition assignment of a scaled diagonal transpose dense matrix-general
   //        transpose dense matrix multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default addition assignment of a scaled diagonal transpose
   // dense matrix-general transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< IsDiagonal<MT4>, Not< IsDiagonal<MT5> > > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t ibegin( ( IsLower<MT5>::value )
                              ?( IsStrictlyLower<MT5>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t iend( ( IsUpper<MT5>::value )
                            ?( IsStrictlyUpper<MT5>::value ? j : j+1UL )
                            :( M ) );
         BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

         const size_t inum( iend - ibegin );
         const size_t ipos( ibegin + ( inum & size_t(-2) ) );

         for( size_t i=ibegin; i<ipos; i+=2UL ) {
            C(i    ,j) += A(i    ,i    ) * B(i    ,j) * scalar;
            C(i+1UL,j) += A(i+1UL,i+1UL) * B(i+1UL,j) * scalar;
         }
         if( ipos < iend ) {
            C(ipos,j) += A(ipos,ipos) * B(ipos,j) * scalar;
         }
      }
   }
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (diagonal/diagonal)***************************
   /*!\brief Default addition assignment of a scaled diagonal transpose dense matrix-diagonal
   //        transpose dense matrix multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default addition assignment of a scaled diagonal transpose
   // dense matrix-diagonal transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< IsDiagonal<MT4>, IsDiagonal<MT5> > >
      selectDefaultAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      for( size_t i=0UL; i<A.rows(); ++i ) {
         C(i,i) += A(i,i) * B(i,i) * scalar;
      }
   }
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (small matrices)******************************
   /*!\brief Default addition assignment of a small scaled transpose dense matrix-transpose dense
   //        matrix multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the addition assignment of a scaled
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectDefaultAddAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Vectorized default addition assignment to row-major dense matrices (small matrices)*********
   /*!\brief Vectorized default addition assignment of a small scaled transpose dense matrix-
   //        transpose dense matrix multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default addition assignment of a scaled transpose
   // dense matrix-transpose dense matrix multiplication expression to a row-major dense matrix.
   // This kernel is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallAddAssignKernel( DenseMatrix<MT3,false>& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT4 );
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT5 );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType_<MT4> );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType_<MT5> );

      const ForwardFunctor fwd;

      if( IsResizable<MT4>::value && !IsResizable<MT5>::value ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         addAssign( ~C, fwd( A * tmp ) * scalar );
      }
      else if( !IsResizable<MT4>::value && IsResizable<MT5>::value ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         addAssign( ~C, fwd( tmp * B ) * scalar );
      }
      else if( B.rows() * B.columns() <= A.rows() * A.columns() ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         addAssign( ~C, fwd( A * tmp ) * scalar );
      }
      else {
         const OppositeType_<MT4> tmp( serial( A ) );
         addAssign( ~C, fwd( tmp * B ) * scalar );
      }
   }
   //**********************************************************************************************

   //**Vectorized default addition assignment to column-major dense matrices (small matrices)******
   /*!\brief Vectorized default addition assignment of a small scaled transpose dense matrix-
   //        transpose dense matrix multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default addition assignment of a scaled transpose
   // dense matrix-transpose dense matrix multiplication expression to a column-major dense matrix.
   // This kernel is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallAddAssignKernel( DenseMatrix<MT3,true>& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT4>::value );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      const size_t ipos( remainder ? ( M & size_t(-SIMDSIZE) ) : M );
      BLAZE_INTERNAL_ASSERT( !remainder || ( M - ( M % SIMDSIZE ) ) == ipos, "Invalid end calculation" );

      const SIMDType factor( set( scalar ) );

      size_t i( 0UL );

      if( IsIntegral<ElementType>::value )
      {
         for( ; !LOW && !UPP && (i+SIMDSIZE*7UL) < ipos; i+=SIMDSIZE*8UL ) {
            for( size_t j=0UL; j<N; ++j )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*8UL, K, ( IsStrictlyUpper<MT5>::value ? j : j+1UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j : j+1UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*8UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType b1( set( B(k,j) ) );
                  xmm1 += A.load(i             ,k) * b1;
                  xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
                  xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
                  xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
                  xmm5 += A.load(i+SIMDSIZE*4UL,k) * b1;
                  xmm6 += A.load(i+SIMDSIZE*5UL,k) * b1;
                  xmm7 += A.load(i+SIMDSIZE*6UL,k) * b1;
                  xmm8 += A.load(i+SIMDSIZE*7UL,k) * b1;
               }

               (~C).store( i             , j, (~C).load(i             ,j) + xmm1 * factor );
               (~C).store( i+SIMDSIZE    , j, (~C).load(i+SIMDSIZE    ,j) + xmm2 * factor );
               (~C).store( i+SIMDSIZE*2UL, j, (~C).load(i+SIMDSIZE*2UL,j) + xmm3 * factor );
               (~C).store( i+SIMDSIZE*3UL, j, (~C).load(i+SIMDSIZE*3UL,j) + xmm4 * factor );
               (~C).store( i+SIMDSIZE*4UL, j, (~C).load(i+SIMDSIZE*4UL,j) + xmm5 * factor );
               (~C).store( i+SIMDSIZE*5UL, j, (~C).load(i+SIMDSIZE*5UL,j) + xmm6 * factor );
               (~C).store( i+SIMDSIZE*6UL, j, (~C).load(i+SIMDSIZE*6UL,j) + xmm7 * factor );
               (~C).store( i+SIMDSIZE*7UL, j, (~C).load(i+SIMDSIZE*7UL,j) + xmm8 * factor );
            }
         }
      }

      for( ; !LOW && !UPP && (i+SIMDSIZE*4UL) < ipos; i+=SIMDSIZE*5UL )
      {
         size_t j( 0UL );

         for( ; (j+2UL) <= N; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*5UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*5UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i             ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
               const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
               const SIMDType a4( A.load(i+SIMDSIZE*3UL,k) );
               const SIMDType a5( A.load(i+SIMDSIZE*4UL,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1  += a1 * b1;
               xmm2  += a2 * b1;
               xmm3  += a3 * b1;
               xmm4  += a4 * b1;
               xmm5  += a5 * b1;
               xmm6  += a1 * b2;
               xmm7  += a2 * b2;
               xmm8  += a3 * b2;
               xmm9  += a4 * b2;
               xmm10 += a5 * b2;
            }

            (~C).store( i             , j    , (~C).load(i             ,j    ) + xmm1  * factor );
            (~C).store( i+SIMDSIZE    , j    , (~C).load(i+SIMDSIZE    ,j    ) + xmm2  * factor );
            (~C).store( i+SIMDSIZE*2UL, j    , (~C).load(i+SIMDSIZE*2UL,j    ) + xmm3  * factor );
            (~C).store( i+SIMDSIZE*3UL, j    , (~C).load(i+SIMDSIZE*3UL,j    ) + xmm4  * factor );
            (~C).store( i+SIMDSIZE*4UL, j    , (~C).load(i+SIMDSIZE*4UL,j    ) + xmm5  * factor );
            (~C).store( i             , j+1UL, (~C).load(i             ,j+1UL) + xmm6  * factor );
            (~C).store( i+SIMDSIZE    , j+1UL, (~C).load(i+SIMDSIZE    ,j+1UL) + xmm7  * factor );
            (~C).store( i+SIMDSIZE*2UL, j+1UL, (~C).load(i+SIMDSIZE*2UL,j+1UL) + xmm8  * factor );
            (~C).store( i+SIMDSIZE*3UL, j+1UL, (~C).load(i+SIMDSIZE*3UL,j+1UL) + xmm9  * factor );
            (~C).store( i+SIMDSIZE*4UL, j+1UL, (~C).load(i+SIMDSIZE*4UL,j+1UL) + xmm10 * factor );
         }

         if( j < N )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*5UL, K ) ):( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 += A.load(i             ,k) * b1;
               xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
               xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
               xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
               xmm5 += A.load(i+SIMDSIZE*4UL,k) * b1;
            }

            (~C).store( i             , j, (~C).load(i             ,j) + xmm1 * factor );
            (~C).store( i+SIMDSIZE    , j, (~C).load(i+SIMDSIZE    ,j) + xmm2 * factor );
            (~C).store( i+SIMDSIZE*2UL, j, (~C).load(i+SIMDSIZE*2UL,j) + xmm3 * factor );
            (~C).store( i+SIMDSIZE*3UL, j, (~C).load(i+SIMDSIZE*3UL,j) + xmm4 * factor );
            (~C).store( i+SIMDSIZE*4UL, j, (~C).load(i+SIMDSIZE*4UL,j) + xmm5 * factor );
         }
      }

      for( ; !LOW && !UPP && (i+SIMDSIZE*3UL) < ipos; i+=SIMDSIZE*4UL )
      {
         size_t j( 0UL );

         for( ; (j+2UL) <= N; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*4UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*4UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i             ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
               const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
               const SIMDType a4( A.load(i+SIMDSIZE*3UL,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a3 * b1;
               xmm4 += a4 * b1;
               xmm5 += a1 * b2;
               xmm6 += a2 * b2;
               xmm7 += a3 * b2;
               xmm8 += a4 * b2;
            }

            (~C).store( i             , j    , (~C).load(i             ,j    ) + xmm1 * factor );
            (~C).store( i+SIMDSIZE    , j    , (~C).load(i+SIMDSIZE    ,j    ) + xmm2 * factor );
            (~C).store( i+SIMDSIZE*2UL, j    , (~C).load(i+SIMDSIZE*2UL,j    ) + xmm3 * factor );
            (~C).store( i+SIMDSIZE*3UL, j    , (~C).load(i+SIMDSIZE*3UL,j    ) + xmm4 * factor );
            (~C).store( i             , j+1UL, (~C).load(i             ,j+1UL) + xmm5 * factor );
            (~C).store( i+SIMDSIZE    , j+1UL, (~C).load(i+SIMDSIZE    ,j+1UL) + xmm6 * factor );
            (~C).store( i+SIMDSIZE*2UL, j+1UL, (~C).load(i+SIMDSIZE*2UL,j+1UL) + xmm7 * factor );
            (~C).store( i+SIMDSIZE*3UL, j+1UL, (~C).load(i+SIMDSIZE*3UL,j+1UL) + xmm8 * factor );
         }

         if( j < N )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*4UL, K ) ):( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 += A.load(i             ,k) * b1;
               xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
               xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
               xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
            }

            (~C).store( i             , j, (~C).load(i             ,j) + xmm1 * factor );
            (~C).store( i+SIMDSIZE    , j, (~C).load(i+SIMDSIZE    ,j) + xmm2 * factor );
            (~C).store( i+SIMDSIZE*2UL, j, (~C).load(i+SIMDSIZE*2UL,j) + xmm3 * factor );
            (~C).store( i+SIMDSIZE*3UL, j, (~C).load(i+SIMDSIZE*3UL,j) + xmm4 * factor );
         }
      }

      for( ; !LOW && !UPP && (i+SIMDSIZE*2UL) < ipos; i+=SIMDSIZE*3UL )
      {
         size_t j( 0UL );

         for( ; (j+2UL) <= N; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*3UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*3UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i             ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
               const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a3 * b1;
               xmm4 += a1 * b2;
               xmm5 += a2 * b2;
               xmm6 += a3 * b2;
            }

            (~C).store( i             , j    , (~C).load(i             ,j    ) + xmm1 * factor );
            (~C).store( i+SIMDSIZE    , j    , (~C).load(i+SIMDSIZE    ,j    ) + xmm2 * factor );
            (~C).store( i+SIMDSIZE*2UL, j    , (~C).load(i+SIMDSIZE*2UL,j    ) + xmm3 * factor );
            (~C).store( i             , j+1UL, (~C).load(i             ,j+1UL) + xmm4 * factor );
            (~C).store( i+SIMDSIZE    , j+1UL, (~C).load(i+SIMDSIZE    ,j+1UL) + xmm5 * factor );
            (~C).store( i+SIMDSIZE*2UL, j+1UL, (~C).load(i+SIMDSIZE*2UL,j+1UL) + xmm6 * factor );
         }

         if( j < N )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*3UL, K ) ):( K ) );

            SIMDType xmm1, xmm2, xmm3;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 += A.load(i             ,k) * b1;
               xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
               xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
            }

            (~C).store( i             , j, (~C).load(i             ,j) + xmm1 * factor );
            (~C).store( i+SIMDSIZE    , j, (~C).load(i+SIMDSIZE    ,j) + xmm2 * factor );
            (~C).store( i+SIMDSIZE*2UL, j, (~C).load(i+SIMDSIZE*2UL,j) + xmm3 * factor );
         }
      }

      for( ; !( LOW && UPP ) && (i+SIMDSIZE) < ipos; i+=SIMDSIZE*2UL )
      {
         const size_t jend( LOW ? min(i+SIMDSIZE*2UL,N) : N );
         size_t j( UPP ? i : 0UL );

         for( ; (j+4UL) <= jend; j+=4UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i         ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               const SIMDType b3( set( B(k,j+2UL) ) );
               const SIMDType b4( set( B(k,j+3UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a1 * b2;
               xmm4 += a2 * b2;
               xmm5 += a1 * b3;
               xmm6 += a2 * b3;
               xmm7 += a1 * b4;
               xmm8 += a2 * b4;
            }

            (~C).store( i         , j    , (~C).load(i         ,j    ) + xmm1 * factor );
            (~C).store( i+SIMDSIZE, j    , (~C).load(i+SIMDSIZE,j    ) + xmm2 * factor );
            (~C).store( i         , j+1UL, (~C).load(i         ,j+1UL) + xmm3 * factor );
            (~C).store( i+SIMDSIZE, j+1UL, (~C).load(i+SIMDSIZE,j+1UL) + xmm4 * factor );
            (~C).store( i         , j+2UL, (~C).load(i         ,j+2UL) + xmm5 * factor );
            (~C).store( i+SIMDSIZE, j+2UL, (~C).load(i+SIMDSIZE,j+2UL) + xmm6 * factor );
            (~C).store( i         , j+3UL, (~C).load(i         ,j+3UL) + xmm7 * factor );
            (~C).store( i+SIMDSIZE, j+3UL, (~C).load(i+SIMDSIZE,j+3UL) + xmm8 * factor );
         }

         for( ; (j+3UL) <= jend; j+=3UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i         ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               const SIMDType b3( set( B(k,j+2UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a1 * b2;
               xmm4 += a2 * b2;
               xmm5 += a1 * b3;
               xmm6 += a2 * b3;
            }

            (~C).store( i         , j    , (~C).load(i         ,j    ) + xmm1 * factor );
            (~C).store( i+SIMDSIZE, j    , (~C).load(i+SIMDSIZE,j    ) + xmm2 * factor );
            (~C).store( i         , j+1UL, (~C).load(i         ,j+1UL) + xmm3 * factor );
            (~C).store( i+SIMDSIZE, j+1UL, (~C).load(i+SIMDSIZE,j+1UL) + xmm4 * factor );
            (~C).store( i         , j+2UL, (~C).load(i         ,j+2UL) + xmm5 * factor );
            (~C).store( i+SIMDSIZE, j+2UL, (~C).load(i+SIMDSIZE,j+2UL) + xmm6 * factor );
         }

         for( ; (j+2UL) <= jend; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i         ,k    ) );
               const SIMDType a2( A.load(i+SIMDSIZE,k    ) );
               const SIMDType a3( A.load(i         ,k+1UL) );
               const SIMDType a4( A.load(i+SIMDSIZE,k+1UL) );
               const SIMDType b1( set( B(k    ,j    ) ) );
               const SIMDType b2( set( B(k    ,j+1UL) ) );
               const SIMDType b3( set( B(k+1UL,j    ) ) );
               const SIMDType b4( set( B(k+1UL,j+1UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a1 * b2;
               xmm4 += a2 * b2;
               xmm5 += a3 * b3;
               xmm6 += a4 * b3;
               xmm7 += a3 * b4;
               xmm8 += a4 * b4;
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i         ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a1 * b2;
               xmm4 += a2 * b2;
            }

            (~C).store( i         , j    , (~C).load(i         ,j    ) + (xmm1+xmm5) * factor );
            (~C).store( i+SIMDSIZE, j    , (~C).load(i+SIMDSIZE,j    ) + (xmm2+xmm6) * factor );
            (~C).store( i         , j+1UL, (~C).load(i         ,j+1UL) + (xmm3+xmm7) * factor );
            (~C).store( i+SIMDSIZE, j+1UL, (~C).load(i+SIMDSIZE,j+1UL) + (xmm4+xmm8) * factor );
         }

         if( j < jend )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*2UL, K ) ):( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType b1( set( B(k    ,j) ) );
               const SIMDType b2( set( B(k+1UL,j) ) );
               xmm1 += A.load(i         ,k    ) * b1;
               xmm2 += A.load(i+SIMDSIZE,k    ) * b1;
               xmm3 += A.load(i         ,k+1UL) * b2;
               xmm4 += A.load(i+SIMDSIZE,k+1UL) * b2;
            }

            for( ; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 += A.load(i         ,k) * b1;
               xmm2 += A.load(i+SIMDSIZE,k) * b1;
            }

            (~C).store( i         , j, (~C).load(i         ,j) + (xmm1+xmm3) * factor );
            (~C).store( i+SIMDSIZE, j, (~C).load(i+SIMDSIZE,j) + (xmm2+xmm4) * factor );
         }
      }

      for( ; i<ipos; i+=SIMDSIZE )
      {
         const size_t jend( LOW && UPP ? min(i+SIMDSIZE,N) : N );
         size_t j( UPP ? i : 0UL );

         for( ; (j+4UL) <= jend; j+=4UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL )
                               :( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i,k    ) );
               const SIMDType a2( A.load(i,k+1UL) );
               xmm1 += a1 * set( B(k    ,j    ) );
               xmm2 += a1 * set( B(k    ,j+1UL) );
               xmm3 += a1 * set( B(k    ,j+2UL) );
               xmm4 += a1 * set( B(k    ,j+3UL) );
               xmm5 += a2 * set( B(k+1UL,j    ) );
               xmm6 += a2 * set( B(k+1UL,j+1UL) );
               xmm7 += a2 * set( B(k+1UL,j+2UL) );
               xmm8 += a2 * set( B(k+1UL,j+3UL) );
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i,k) );
               xmm1 += a1 * set( B(k,j    ) );
               xmm2 += a1 * set( B(k,j+1UL) );
               xmm3 += a1 * set( B(k,j+2UL) );
               xmm4 += a1 * set( B(k,j+3UL) );
            }

            (~C).store( i, j    , (~C).load(i,j    ) + (xmm1+xmm5) * factor );
            (~C).store( i, j+1UL, (~C).load(i,j+1UL) + (xmm2+xmm6) * factor );
            (~C).store( i, j+2UL, (~C).load(i,j+2UL) + (xmm3+xmm7) * factor );
            (~C).store( i, j+3UL, (~C).load(i,j+3UL) + (xmm4+xmm8) * factor );
         }

         for( ; (j+3UL) <= jend; j+=3UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL )
                               :( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i,k    ) );
               const SIMDType a2( A.load(i,k+1UL) );
               xmm1 += a1 * set( B(k    ,j    ) );
               xmm2 += a1 * set( B(k    ,j+1UL) );
               xmm3 += a1 * set( B(k    ,j+2UL) );
               xmm4 += a2 * set( B(k+1UL,j    ) );
               xmm5 += a2 * set( B(k+1UL,j+1UL) );
               xmm6 += a2 * set( B(k+1UL,j+2UL) );
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i,k) );
               xmm1 += a1 * set( B(k,j    ) );
               xmm2 += a1 * set( B(k,j+1UL) );
               xmm3 += a1 * set( B(k,j+2UL) );
            }

            (~C).store( i, j    , (~C).load(i,j    ) + (xmm1+xmm4) * factor );
            (~C).store( i, j+1UL, (~C).load(i,j+1UL) + (xmm2+xmm5) * factor );
            (~C).store( i, j+2UL, (~C).load(i,j+2UL) + (xmm3+xmm6) * factor );
         }

         for( ; (j+2UL) <= jend; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL )
                               :( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i,k    ) );
               const SIMDType a2( A.load(i,k+1UL) );
               xmm1 += a1 * set( B(k    ,j    ) );
               xmm2 += a1 * set( B(k    ,j+1UL) );
               xmm3 += a2 * set( B(k+1UL,j    ) );
               xmm4 += a2 * set( B(k+1UL,j+1UL) );
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i,k) );
               xmm1 += a1 * set( B(k,j    ) );
               xmm2 += a1 * set( B(k,j+1UL) );
            }

            (~C).store( i, j    , (~C).load(i,j    ) + (xmm1+xmm3) * factor );
            (~C).store( i, j+1UL, (~C).load(i,j+1UL) + (xmm2+xmm4) * factor );
         }

         if( j < jend )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );

            SIMDType xmm1, xmm2;
            size_t k( kbegin );

            for( ; (k+2UL) <= K; k+=2UL ) {
               xmm1 += A.load(i,k    ) * set( B(k    ,j) );
               xmm2 += A.load(i,k+1UL) * set( B(k+1UL,j) );
            }

            for( ; k<K; ++k ) {
               xmm1 += A.load(i,k) * set( B(k,j) );
            }

            (~C).store( i, j, (~C).load(i,j) + (xmm1+xmm2) * factor );
         }
      }

      for( ; remainder && i<M; ++i )
      {
         const size_t jend( LOW ? i+1UL : N );
         size_t j( UPP ? i : 0UL );

         for( ; (j+2UL) <= jend; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL )
                               :( K ) );

            ElementType value1{};
            ElementType value2{};

            for( size_t k=kbegin; k<kend; ++k ) {
               value1 += A(i,k) * B(k,j    );
               value2 += A(i,k) * B(k,j+1UL);
            }

            (~C)(i,j    ) += value1 * scalar;
            (~C)(i,j+1UL) += value2 * scalar;
         }

         if( j < jend )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );

            ElementType value{};

            for( size_t k=kbegin; k<K; ++k ) {
               value += A(i,k) * B(k,j);
            }

            (~C)(i,j) += value * scalar;
         }
      }
   }
   //**********************************************************************************************

   //**Default addition assignment to dense matrices (large matrices)******************************
   /*!\brief Default addition assignment of a large scaled transpose dense matrix-transpose dense
   //        matrix multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the addition assignment of a scaled
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectLargeAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectDefaultAddAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Vectorized default addition assignment to dense matrices (large matrices)*******************
   /*!\brief Vectorized default addition assignment of a large scaled transpose dense matrix-
   //        transpose dense matrix multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default addition assignment of a scaled transpose
   // dense matrix-transpose dense matrix multiplication expression to a dense matrix. This
   // kernel is optimized for large matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectLargeAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      if( LOW )
         lmmm( C, A, B, scalar, ST2(1) );
      else if( UPP )
         ummm( C, A, B, scalar, ST2(1) );
      else
         mmm( C, A, B, scalar, ST2(1) );
   }
   //**********************************************************************************************

   //**BLAS-based addition assignment to dense matrices (default)**********************************
   /*!\brief Default addition assignment of a scaled transpose dense matrix-transpose dense matrix
   //        multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the addition assignment of a large
   // scaled transpose dense matrix-transpose dense matrix multiplication expression to a dense
   // matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseBlasKernel<MT3,MT4,MT5,ST2> >
      selectBlasAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectLargeAddAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**BLAS-based addition assignment to dense matrices********************************************
#if BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION
   /*!\brief BLAS-based addition assignment of a scaled transpose dense matrix-transpose dense
   //        matrix multiplication (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function performs the scaled transpose dense matrix-transpose dense matrix multiplication
   // based on the according BLAS functionality.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseBlasKernel<MT3,MT4,MT5,ST2> >
      selectBlasAddAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      using ET = ElementType_<MT3>;

      if( IsTriangular<MT4>::value ) {
         ResultType_<MT3> tmp( serial( B ) );
         trmm( tmp, A, CblasLeft, ( IsLower<MT4>::value )?( CblasLower ):( CblasUpper ), ET(scalar) );
         addAssign( C, tmp );
      }
      else if( IsTriangular<MT5>::value ) {
         ResultType_<MT3> tmp( serial( A ) );
         trmm( tmp, B, CblasRight, ( IsLower<MT5>::value )?( CblasLower ):( CblasUpper ), ET(scalar) );
         addAssign( C, tmp );
      }
      else {
         gemm( C, A, B, ET(scalar), ET(1) );
      }
   }
#endif
   //**********************************************************************************************

   //**Restructuring addition assignment to row-major matrices*************************************
   /*!\brief Restructuring addition assignment of a scaled transpose dense matrix-transpose dense
   //        matrix multiplication to a row-major matrix (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side scaled multiplication expression to be added.
   // \return void
   //
   // This function implements the symmetry-based restructuring addition assignment of a scaled
   // transpose dense matrix-transpose dense matrix multiplication expression to a row-major
   // matrix. Due to the explicit application of the SFINAE principle this function can only be
   // selected by the compiler in case the symmetry of either of the two matrix operands can be
   // exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      addAssign( Matrix<MT,false>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         addAssign( ~lhs, fwd( trans( left ) * trans( right ) ) * rhs.scalar_ );
      else if( IsSymmetric<MT1>::value )
         addAssign( ~lhs, fwd( trans( left ) * right ) * rhs.scalar_ );
      else
         addAssign( ~lhs, fwd( left * trans( right ) ) * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Addition assignment to sparse matrices******************************************************
   // No special implementation for the addition assignment to sparse matrices.
   //**********************************************************************************************

   //**Subtraction assignment to dense matrices****************************************************
   /*!\brief Subtraction assignment of a scaled transpose dense matrix-transpose dense matrix
   //        multiplication to a dense matrix (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the performance optimized subtraction assignment of a scaled transpose
   // dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline DisableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      subAssign( DenseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || left.columns() == 0UL ) {
         return;
      }

      LT A( serial( left  ) );  // Evaluation of the left-hand side dense matrix operand
      RT B( serial( right ) );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == left.rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == left.columns()  , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == right.rows()    , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == right.columns() , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns(), "Invalid number of columns" );

      DMatScalarMultExpr::selectSubAssignKernel( ~lhs, A, B, rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Subtraction assignment to dense matrices (kernel selection)*********************************
   /*!\brief Selection of the kernel for a subtraction assignment of a scaled transpose dense
   //        matrix-transpose dense matrix multiplication to a dense matrix (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline void selectSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      if( ( IsDiagonal<MT4>::value ) ||
          ( !BLAZE_DEBUG_MODE && A.rows() <= SIMDSIZE*10UL ) ||
          ( C.rows() * C.columns() < TDMATTDMATMULT_THRESHOLD ) )
         selectSmallSubAssignKernel( C, A, B, scalar );
      else
         selectBlasSubAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (general/general)**************************
   /*!\brief Default subtraction assignment of a scaled general transpose dense matrix-general
   //        transpose dense matrix multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default subtraction assignment of a scaled general transpose
   // dense matrix-general transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, Not< IsDiagonal<MT5> > > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      const ResultType tmp( serial( A * B * scalar ) );
      subAssign( C, tmp );
   }
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (general/diagonal)*************************
   /*!\brief Default subtraction assignment of a scaled general transpose dense matrix-diagonal
   //        transpose dense matrix multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default subtraction assignment of a scaled general transpose
   // dense matrix-diagonal transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< Not< IsDiagonal<MT4> >, IsDiagonal<MT5> > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t ibegin( ( IsLower<MT4>::value )
                              ?( IsStrictlyLower<MT4>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t iend( ( IsUpper<MT4>::value )
                            ?( IsStrictlyUpper<MT4>::value ? j : j+1UL )
                            :( M ) );
         BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

         const size_t inum( iend - ibegin );
         const size_t ipos( ibegin + ( inum & size_t(-2) ) );

         for( size_t i=ibegin; i<ipos; i+=2UL ) {
            C(i    ,j) -= A(i    ,j) * B(j,j) * scalar;
            C(i+1UL,j) -= A(i+1UL,j) * B(j,j) * scalar;
         }
         if( ipos < iend ) {
            C(ipos,j) -= A(ipos,j) * B(j,j) * scalar;
         }
      }
   }
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (diagonal/general)*************************
   /*!\brief Default subtraction assignment of a scaled diagonal transpose dense matrix-general
   //        transpose dense matrix multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default subtraction assignment of a scaled diagonal transpose
   // dense matrix-general transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< IsDiagonal<MT4>, Not< IsDiagonal<MT5> > > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );

      for( size_t j=0UL; j<N; ++j )
      {
         const size_t ibegin( ( IsLower<MT5>::value )
                              ?( IsStrictlyLower<MT5>::value ? j+1UL : j )
                              :( 0UL ) );
         const size_t iend( ( IsUpper<MT5>::value )
                            ?( IsStrictlyUpper<MT5>::value ? j : j+1UL )
                            :( M ) );
         BLAZE_INTERNAL_ASSERT( ibegin <= iend, "Invalid loop indices detected" );

         const size_t inum( iend - ibegin );
         const size_t ipos( ibegin + ( inum & size_t(-2) ) );

         for( size_t i=ibegin; i<ipos; i+=2UL ) {
            C(i    ,j) -= A(i    ,i    ) * B(i    ,j) * scalar;
            C(i+1UL,j) -= A(i+1UL,i+1UL) * B(i+1UL,j) * scalar;
         }
         if( ipos < iend ) {
            C(ipos,j) -= A(ipos,ipos) * B(ipos,j) * scalar;
         }
      }
   }
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (diagonal/diagonal)************************
   /*!\brief Default subtraction assignment of a scaled diagonal transpose dense matrix-diagonal
   //        transpose dense matrix multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the default subtraction assignment of a scaled diagonal transpose
   // dense matrix-diagonal transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< And< IsDiagonal<MT4>, IsDiagonal<MT5> > >
      selectDefaultSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT3 );

      for( size_t i=0UL; i<A.rows(); ++i ) {
         C(i,i) -= A(i,i) * B(i,i) * scalar;
      }
   }
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (small matrices)***************************
   /*!\brief Default subtraction assignment of a small scaled transpose dense matrix-transpose
   //        dense matrix multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the subtraction assignment of a scaled
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectDefaultSubAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Vectorized default subtraction assignment to row-major dense matrices (small matrices)******
   /*!\brief Default subtraction assignment of a small scaled transpose dense matrix-transpose
   //        dense matrix multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default subtraction assignment of a scaled transpose
   // dense matrix-transpose dense matrix multiplication expression to a row-major dense matrix.
   // This kernel is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallSubAssignKernel( DenseMatrix<MT3,false>& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT4 );
      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT5 );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType_<MT4> );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType_<MT5> );

      const ForwardFunctor fwd;

      if( IsResizable<MT4>::value && !IsResizable<MT5>::value ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         subAssign( ~C, fwd( A * tmp ) * scalar );
      }
      else if( !IsResizable<MT4>::value && IsResizable<MT5>::value ) {
         const OppositeType_<MT4> tmp( serial( A ) );
         subAssign( ~C, fwd( tmp * B ) * scalar );
      }
      else if( B.rows() * B.columns() <= A.rows() * A.columns() ) {
         const OppositeType_<MT5> tmp( serial( B ) );
         subAssign( ~C, fwd( A * tmp ) * scalar );
      }
      else {
         const OppositeType_<MT4> tmp( serial( A ) );
         subAssign( ~C, fwd( tmp * B ) * scalar );
      }
   }
   //**********************************************************************************************

   //**Vectorized default subtraction assignment to column-major dense matrices (small matrices)***
   /*!\brief Default subtraction assignment of a small scaled transpose dense matrix-transpose
   //        dense matrix multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default subtraction assignment of a scaled transpose
   // dense matrix-transpose dense matrix multiplication expression to a column-major dense matrix.
   // This kernel is optimized for small matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectSmallSubAssignKernel( DenseMatrix<MT3,true>& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      constexpr bool remainder( !IsPadded<MT3>::value || !IsPadded<MT4>::value );

      const size_t M( A.rows()    );
      const size_t N( B.columns() );
      const size_t K( A.columns() );

      BLAZE_INTERNAL_ASSERT( !( LOW || UPP ) || ( M == N ), "Broken invariant detected" );

      const size_t ipos( remainder ? ( M & size_t(-SIMDSIZE) ) : M );
      BLAZE_INTERNAL_ASSERT( !remainder || ( M - ( M % SIMDSIZE ) ) == ipos, "Invalid end calculation" );

      const SIMDType factor( set( scalar ) );

      size_t i( 0UL );

      if( IsIntegral<ElementType>::value )
      {
         for( ; !LOW && !UPP && (i+SIMDSIZE*7UL) < ipos; i+=SIMDSIZE*8UL ) {
            for( size_t j=0UL; j<N; ++j )
            {
               const size_t kbegin( ( IsLower<MT5>::value )
                                    ?( ( IsUpper<MT4>::value )
                                       ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                       :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                    :( IsUpper<MT4>::value ? i : 0UL ) );
               const size_t kend( ( IsUpper<MT5>::value )
                                  ?( ( IsLower<MT4>::value )
                                     ?( min( i+SIMDSIZE*8UL, K, ( IsStrictlyUpper<MT5>::value ? j : j+1UL ) ) )
                                     :( IsStrictlyUpper<MT5>::value ? j : j+1UL ) )
                                  :( IsLower<MT4>::value ? min( i+SIMDSIZE*8UL, K ) : K ) );

               SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

               for( size_t k=kbegin; k<kend; ++k ) {
                  const SIMDType b1( set( B(k,j) ) );
                  xmm1 += A.load(i             ,k) * b1;
                  xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
                  xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
                  xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
                  xmm5 += A.load(i+SIMDSIZE*4UL,k) * b1;
                  xmm6 += A.load(i+SIMDSIZE*5UL,k) * b1;
                  xmm7 += A.load(i+SIMDSIZE*6UL,k) * b1;
                  xmm8 += A.load(i+SIMDSIZE*7UL,k) * b1;
               }

               (~C).store( i             , j, (~C).load(i             ,j) - xmm1 * factor );
               (~C).store( i+SIMDSIZE    , j, (~C).load(i+SIMDSIZE    ,j) - xmm2 * factor );
               (~C).store( i+SIMDSIZE*2UL, j, (~C).load(i+SIMDSIZE*2UL,j) - xmm3 * factor );
               (~C).store( i+SIMDSIZE*3UL, j, (~C).load(i+SIMDSIZE*3UL,j) - xmm4 * factor );
               (~C).store( i+SIMDSIZE*4UL, j, (~C).load(i+SIMDSIZE*4UL,j) - xmm5 * factor );
               (~C).store( i+SIMDSIZE*5UL, j, (~C).load(i+SIMDSIZE*5UL,j) - xmm6 * factor );
               (~C).store( i+SIMDSIZE*6UL, j, (~C).load(i+SIMDSIZE*6UL,j) - xmm7 * factor );
               (~C).store( i+SIMDSIZE*7UL, j, (~C).load(i+SIMDSIZE*7UL,j) - xmm8 * factor );
            }
         }
      }

      for( ; !LOW && !UPP && (i+SIMDSIZE*4UL) < ipos; i+=SIMDSIZE*5UL )
      {
         size_t j( 0UL );

         for( ; (j+2UL) <= N; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*5UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*5UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i             ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
               const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
               const SIMDType a4( A.load(i+SIMDSIZE*3UL,k) );
               const SIMDType a5( A.load(i+SIMDSIZE*4UL,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1  += a1 * b1;
               xmm2  += a2 * b1;
               xmm3  += a3 * b1;
               xmm4  += a4 * b1;
               xmm5  += a5 * b1;
               xmm6  += a1 * b2;
               xmm7  += a2 * b2;
               xmm8  += a3 * b2;
               xmm9  += a4 * b2;
               xmm10 += a5 * b2;
            }

            (~C).store( i             , j    , (~C).load(i             ,j    ) - xmm1  * factor );
            (~C).store( i+SIMDSIZE    , j    , (~C).load(i+SIMDSIZE    ,j    ) - xmm2  * factor );
            (~C).store( i+SIMDSIZE*2UL, j    , (~C).load(i+SIMDSIZE*2UL,j    ) - xmm3  * factor );
            (~C).store( i+SIMDSIZE*3UL, j    , (~C).load(i+SIMDSIZE*3UL,j    ) - xmm4  * factor );
            (~C).store( i+SIMDSIZE*4UL, j    , (~C).load(i+SIMDSIZE*4UL,j    ) - xmm5  * factor );
            (~C).store( i             , j+1UL, (~C).load(i             ,j+1UL) - xmm6  * factor );
            (~C).store( i+SIMDSIZE    , j+1UL, (~C).load(i+SIMDSIZE    ,j+1UL) - xmm7  * factor );
            (~C).store( i+SIMDSIZE*2UL, j+1UL, (~C).load(i+SIMDSIZE*2UL,j+1UL) - xmm8  * factor );
            (~C).store( i+SIMDSIZE*3UL, j+1UL, (~C).load(i+SIMDSIZE*3UL,j+1UL) - xmm9  * factor );
            (~C).store( i+SIMDSIZE*4UL, j+1UL, (~C).load(i+SIMDSIZE*4UL,j+1UL) - xmm10 * factor );
         }

         if( j < N )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*5UL, K ) ):( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 += A.load(i             ,k) * b1;
               xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
               xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
               xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
               xmm5 += A.load(i+SIMDSIZE*4UL,k) * b1;
            }

            (~C).store( i             , j, (~C).load(i             ,j) - xmm1 * factor );
            (~C).store( i+SIMDSIZE    , j, (~C).load(i+SIMDSIZE    ,j) - xmm2 * factor );
            (~C).store( i+SIMDSIZE*2UL, j, (~C).load(i+SIMDSIZE*2UL,j) - xmm3 * factor );
            (~C).store( i+SIMDSIZE*3UL, j, (~C).load(i+SIMDSIZE*3UL,j) - xmm4 * factor );
            (~C).store( i+SIMDSIZE*4UL, j, (~C).load(i+SIMDSIZE*4UL,j) - xmm5 * factor );
         }
      }

      for( ; !LOW && !UPP && (i+SIMDSIZE*3UL) < ipos; i+=SIMDSIZE*4UL )
      {
         size_t j( 0UL );

         for( ; (j+2UL) <= N; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*4UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*4UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i             ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
               const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
               const SIMDType a4( A.load(i+SIMDSIZE*3UL,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a3 * b1;
               xmm4 += a4 * b1;
               xmm5 += a1 * b2;
               xmm6 += a2 * b2;
               xmm7 += a3 * b2;
               xmm8 += a4 * b2;
            }

            (~C).store( i             , j    , (~C).load(i             ,j    ) - xmm1 * factor );
            (~C).store( i+SIMDSIZE    , j    , (~C).load(i+SIMDSIZE    ,j    ) - xmm2 * factor );
            (~C).store( i+SIMDSIZE*2UL, j    , (~C).load(i+SIMDSIZE*2UL,j    ) - xmm3 * factor );
            (~C).store( i+SIMDSIZE*3UL, j    , (~C).load(i+SIMDSIZE*3UL,j    ) - xmm4 * factor );
            (~C).store( i             , j+1UL, (~C).load(i             ,j+1UL) - xmm5 * factor );
            (~C).store( i+SIMDSIZE    , j+1UL, (~C).load(i+SIMDSIZE    ,j+1UL) - xmm6 * factor );
            (~C).store( i+SIMDSIZE*2UL, j+1UL, (~C).load(i+SIMDSIZE*2UL,j+1UL) - xmm7 * factor );
            (~C).store( i+SIMDSIZE*3UL, j+1UL, (~C).load(i+SIMDSIZE*3UL,j+1UL) - xmm8 * factor );
         }

         if( j < N )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*4UL, K ) ):( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 += A.load(i             ,k) * b1;
               xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
               xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
               xmm4 += A.load(i+SIMDSIZE*3UL,k) * b1;
            }

            (~C).store( i             , j, (~C).load(i             ,j) - xmm1 * factor );
            (~C).store( i+SIMDSIZE    , j, (~C).load(i+SIMDSIZE    ,j) - xmm2 * factor );
            (~C).store( i+SIMDSIZE*2UL, j, (~C).load(i+SIMDSIZE*2UL,j) - xmm3 * factor );
            (~C).store( i+SIMDSIZE*3UL, j, (~C).load(i+SIMDSIZE*3UL,j) - xmm4 * factor );
         }
      }

      for( ; !LOW && !UPP && (i+SIMDSIZE*2UL) < ipos; i+=SIMDSIZE*3UL )
      {
         size_t j( 0UL );

         for( ; (j+2UL) <= N; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*3UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*3UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i             ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE    ,k) );
               const SIMDType a3( A.load(i+SIMDSIZE*2UL,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a3 * b1;
               xmm4 += a1 * b2;
               xmm5 += a2 * b2;
               xmm6 += a3 * b2;
            }

            (~C).store( i             , j    , (~C).load(i             ,j    ) - xmm1 * factor );
            (~C).store( i+SIMDSIZE    , j    , (~C).load(i+SIMDSIZE    ,j    ) - xmm2 * factor );
            (~C).store( i+SIMDSIZE*2UL, j    , (~C).load(i+SIMDSIZE*2UL,j    ) - xmm3 * factor );
            (~C).store( i             , j+1UL, (~C).load(i             ,j+1UL) - xmm4 * factor );
            (~C).store( i+SIMDSIZE    , j+1UL, (~C).load(i+SIMDSIZE    ,j+1UL) - xmm5 * factor );
            (~C).store( i+SIMDSIZE*2UL, j+1UL, (~C).load(i+SIMDSIZE*2UL,j+1UL) - xmm6 * factor );
         }

         if( j < N )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*3UL, K ) ):( K ) );

            SIMDType xmm1, xmm2, xmm3;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 += A.load(i             ,k) * b1;
               xmm2 += A.load(i+SIMDSIZE    ,k) * b1;
               xmm3 += A.load(i+SIMDSIZE*2UL,k) * b1;
            }

            (~C).store( i             , j, (~C).load(i             ,j) - xmm1 * factor );
            (~C).store( i+SIMDSIZE    , j, (~C).load(i+SIMDSIZE    ,j) - xmm2 * factor );
            (~C).store( i+SIMDSIZE*2UL, j, (~C).load(i+SIMDSIZE*2UL,j) - xmm3 * factor );
         }
      }

      for( ; !( LOW && UPP ) && (i+SIMDSIZE) < ipos; i+=SIMDSIZE*2UL )
      {
         const size_t jend( LOW ? min(i+SIMDSIZE*2UL,N) : N );
         size_t j( UPP ? i : 0UL );

         for( ; (j+4UL) <= jend; j+=4UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i         ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               const SIMDType b3( set( B(k,j+2UL) ) );
               const SIMDType b4( set( B(k,j+3UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a1 * b2;
               xmm4 += a2 * b2;
               xmm5 += a1 * b3;
               xmm6 += a2 * b3;
               xmm7 += a1 * b4;
               xmm8 += a2 * b4;
            }

            (~C).store( i         , j    , (~C).load(i         ,j    ) - xmm1 * factor );
            (~C).store( i+SIMDSIZE, j    , (~C).load(i+SIMDSIZE,j    ) - xmm2 * factor );
            (~C).store( i         , j+1UL, (~C).load(i         ,j+1UL) - xmm3 * factor );
            (~C).store( i+SIMDSIZE, j+1UL, (~C).load(i+SIMDSIZE,j+1UL) - xmm4 * factor );
            (~C).store( i         , j+2UL, (~C).load(i         ,j+2UL) - xmm5 * factor );
            (~C).store( i+SIMDSIZE, j+2UL, (~C).load(i+SIMDSIZE,j+2UL) - xmm6 * factor );
            (~C).store( i         , j+3UL, (~C).load(i         ,j+3UL) - xmm7 * factor );
            (~C).store( i+SIMDSIZE, j+3UL, (~C).load(i+SIMDSIZE,j+3UL) - xmm8 * factor );
         }

         for( ; (j+3UL) <= jend; j+=3UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;

            for( size_t k=kbegin; k<kend; ++k ) {
               const SIMDType a1( A.load(i         ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               const SIMDType b3( set( B(k,j+2UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a1 * b2;
               xmm4 += a2 * b2;
               xmm5 += a1 * b3;
               xmm6 += a2 * b3;
            }

            (~C).store( i         , j    , (~C).load(i         ,j    ) - xmm1 * factor );
            (~C).store( i+SIMDSIZE, j    , (~C).load(i+SIMDSIZE,j    ) - xmm2 * factor );
            (~C).store( i         , j+1UL, (~C).load(i         ,j+1UL) - xmm3 * factor );
            (~C).store( i+SIMDSIZE, j+1UL, (~C).load(i+SIMDSIZE,j+1UL) - xmm4 * factor );
            (~C).store( i         , j+2UL, (~C).load(i         ,j+2UL) - xmm5 * factor );
            (~C).store( i+SIMDSIZE, j+2UL, (~C).load(i+SIMDSIZE,j+2UL) - xmm6 * factor );
         }

         for( ; (j+2UL) <= jend; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( ( IsLower<MT4>::value )
                                  ?( min( i+SIMDSIZE*2UL, K, ( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) ) )
                                  :( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL ) )
                               :( IsLower<MT4>::value ? min( i+SIMDSIZE*2UL, K ) : K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i         ,k    ) );
               const SIMDType a2( A.load(i+SIMDSIZE,k    ) );
               const SIMDType a3( A.load(i         ,k+1UL) );
               const SIMDType a4( A.load(i+SIMDSIZE,k+1UL) );
               const SIMDType b1( set( B(k    ,j    ) ) );
               const SIMDType b2( set( B(k    ,j+1UL) ) );
               const SIMDType b3( set( B(k+1UL,j    ) ) );
               const SIMDType b4( set( B(k+1UL,j+1UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a1 * b2;
               xmm4 += a2 * b2;
               xmm5 += a3 * b3;
               xmm6 += a4 * b3;
               xmm7 += a3 * b4;
               xmm8 += a4 * b4;
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i         ,k) );
               const SIMDType a2( A.load(i+SIMDSIZE,k) );
               const SIMDType b1( set( B(k,j    ) ) );
               const SIMDType b2( set( B(k,j+1UL) ) );
               xmm1 += a1 * b1;
               xmm2 += a2 * b1;
               xmm3 += a1 * b2;
               xmm4 += a2 * b2;
            }

            (~C).store( i         , j    , (~C).load(i         ,j    ) - (xmm1+xmm5) * factor );
            (~C).store( i+SIMDSIZE, j    , (~C).load(i+SIMDSIZE,j    ) - (xmm2+xmm6) * factor );
            (~C).store( i         , j+1UL, (~C).load(i         ,j+1UL) - (xmm3+xmm7) * factor );
            (~C).store( i+SIMDSIZE, j+1UL, (~C).load(i+SIMDSIZE,j+1UL) - (xmm4+xmm8) * factor );
         }

         if( j < jend )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsLower<MT4>::value )?( min( i+SIMDSIZE*2UL, K ) ):( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType b1( set( B(k    ,j) ) );
               const SIMDType b2( set( B(k+1UL,j) ) );
               xmm1 += A.load(i         ,k    ) * b1;
               xmm2 += A.load(i+SIMDSIZE,k    ) * b1;
               xmm3 += A.load(i         ,k+1UL) * b2;
               xmm4 += A.load(i+SIMDSIZE,k+1UL) * b2;
            }

            for( ; k<kend; ++k ) {
               const SIMDType b1( set( B(k,j) ) );
               xmm1 += A.load(i         ,k) * b1;
               xmm2 += A.load(i+SIMDSIZE,k) * b1;
            }

            (~C).store( i         , j, (~C).load(i         ,j) - (xmm1+xmm3) * factor );
            (~C).store( i+SIMDSIZE, j, (~C).load(i+SIMDSIZE,j) - (xmm2+xmm4) * factor );
         }
      }

      for( ; i<ipos; i+=SIMDSIZE )
      {
         const size_t jend( LOW && UPP ? min(i+SIMDSIZE,N) : N );
         size_t j( UPP ? i : 0UL );

         for( ; (j+4UL) <= jend; j+=4UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+3UL : j+4UL )
                               :( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i,k    ) );
               const SIMDType a2( A.load(i,k+1UL) );
               xmm1 += a1 * set( B(k    ,j    ) );
               xmm2 += a1 * set( B(k    ,j+1UL) );
               xmm3 += a1 * set( B(k    ,j+2UL) );
               xmm4 += a1 * set( B(k    ,j+3UL) );
               xmm5 += a2 * set( B(k+1UL,j    ) );
               xmm6 += a2 * set( B(k+1UL,j+1UL) );
               xmm7 += a2 * set( B(k+1UL,j+2UL) );
               xmm8 += a2 * set( B(k+1UL,j+3UL) );
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i,k) );
               xmm1 += a1 * set( B(k,j    ) );
               xmm2 += a1 * set( B(k,j+1UL) );
               xmm3 += a1 * set( B(k,j+2UL) );
               xmm4 += a1 * set( B(k,j+3UL) );
            }

            (~C).store( i, j    , (~C).load(i,j    ) - (xmm1+xmm5) * factor );
            (~C).store( i, j+1UL, (~C).load(i,j+1UL) - (xmm2+xmm6) * factor );
            (~C).store( i, j+2UL, (~C).load(i,j+2UL) - (xmm3+xmm7) * factor );
            (~C).store( i, j+3UL, (~C).load(i,j+3UL) - (xmm4+xmm8) * factor );
         }

         for( ; (j+3UL) <= jend; j+=3UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+2UL : j+3UL )
                               :( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i,k    ) );
               const SIMDType a2( A.load(i,k+1UL) );
               xmm1 += a1 * set( B(k    ,j    ) );
               xmm2 += a1 * set( B(k    ,j+1UL) );
               xmm3 += a1 * set( B(k    ,j+2UL) );
               xmm4 += a2 * set( B(k+1UL,j    ) );
               xmm5 += a2 * set( B(k+1UL,j+1UL) );
               xmm6 += a2 * set( B(k+1UL,j+2UL) );
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i,k) );
               xmm1 += a1 * set( B(k,j    ) );
               xmm2 += a1 * set( B(k,j+1UL) );
               xmm3 += a1 * set( B(k,j+2UL) );
            }

            (~C).store( i, j    , (~C).load(i,j    ) - (xmm1+xmm4) * factor );
            (~C).store( i, j+1UL, (~C).load(i,j+1UL) - (xmm2+xmm5) * factor );
            (~C).store( i, j+2UL, (~C).load(i,j+2UL) - (xmm3+xmm6) * factor );
         }

         for( ; (j+2UL) <= jend; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL )
                               :( K ) );

            SIMDType xmm1, xmm2, xmm3, xmm4;
            size_t k( kbegin );

            for( ; (k+2UL) <= kend; k+=2UL ) {
               const SIMDType a1( A.load(i,k    ) );
               const SIMDType a2( A.load(i,k+1UL) );
               xmm1 += a1 * set( B(k    ,j    ) );
               xmm2 += a1 * set( B(k    ,j+1UL) );
               xmm3 += a2 * set( B(k+1UL,j    ) );
               xmm4 += a2 * set( B(k+1UL,j+1UL) );
            }

            for( ; k<kend; ++k ) {
               const SIMDType a1( A.load(i,k) );
               xmm1 += a1 * set( B(k,j    ) );
               xmm2 += a1 * set( B(k,j+1UL) );
            }

            (~C).store( i, j    , (~C).load(i,j    ) - (xmm1+xmm3) * factor );
            (~C).store( i, j+1UL, (~C).load(i,j+1UL) - (xmm2+xmm4) * factor );
         }

         if( j < jend )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );

            SIMDType xmm1, xmm2;
            size_t k( kbegin );

            for( ; (k+2UL) <= K; k+=2UL ) {
               xmm1 += A.load(i,k    ) * set( B(k   ,j) );
               xmm2 += A.load(i,k+1UL) * set( B(k+1UL,j) );
            }

            for( ; k<K; ++k ) {
               xmm1 += A.load(i,k) * set( B(k,j) );
            }

            (~C).store( i, j, (~C).load(i,j) - (xmm1+xmm2) * factor );
         }
      }

      for( ; remainder && i<M; ++i )
      {
         const size_t jend( LOW ? i+1UL : N );
         size_t j( UPP ? i : 0UL );

         for( ; (j+2UL) <= jend; j+=2UL )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );
            const size_t kend( ( IsUpper<MT5>::value )
                               ?( IsStrictlyUpper<MT5>::value ? j+1UL : j+2UL )
                               :( K ) );

            ElementType value1{};
            ElementType value2{};

            for( size_t k=kbegin; k<kend; ++k ) {
               value1 += A(i,k) * B(k,j    );
               value2 += A(i,k) * B(k,j+1UL);
            }

            (~C)(i,j    ) -= value1 * scalar;
            (~C)(i,j+1UL) -= value2 * scalar;
         }

         if( j < jend )
         {
            const size_t kbegin( ( IsLower<MT5>::value )
                                 ?( ( IsUpper<MT4>::value )
                                    ?( max( i, ( IsStrictlyLower<MT5>::value ? j+1UL : j ) ) )
                                    :( IsStrictlyLower<MT5>::value ? j+1UL : j ) )
                                 :( IsUpper<MT4>::value ? i : 0UL ) );

            ElementType value{};

            for( size_t k=kbegin; k<K; ++k ) {
               value += A(i,k) * B(k,j);
            }

            (~C)(i,j) -= value * scalar;
         }
      }
   }
   //**********************************************************************************************

   //**Default subtraction assignment to dense matrices (large matrices)***************************
   /*!\brief Default subtraction assignment of a large scaled transpose dense matrix-transpose
   //        dense matrix multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the subtraction assignment of a scaled
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectLargeSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectDefaultSubAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**Vectorized default subtraction assignment to dense matrices (large matrices)****************
   /*!\brief Default subtraction assignment of a large scaled transpose dense matrix-transpose
   //        dense matrix multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function implements the vectorized default subtraction assignment of a scaled transpose
   // dense matrix-transpose dense matrix multiplication expression to a dense matrix. This kernel
   // is optimized for large matrices.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseVectorizedDefaultKernel<MT3,MT4,MT5,ST2> >
      selectLargeSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      if( LOW )
         lmmm( C, A, B, -scalar, ST2(1) );
      else if( UPP )
         ummm( C, A, B, -scalar, ST2(1) );
      else
         mmm( C, A, B, -scalar, ST2(1) );
   }
   //**********************************************************************************************

   //**BLAS-based subtraction assignment to dense matrices (default)*******************************
   /*!\brief Default subtraction assignment of a scaled transpose dense matrix-transpose dense
   //        matrix multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function relays to the default implementation of the subtraction assignment of a large
   // scaled transpose dense matrix-transpose dense matrix multiplication expression to a dense
   // matrix.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline DisableIf_< UseBlasKernel<MT3,MT4,MT5,ST2> >
      selectBlasSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      selectLargeSubAssignKernel( C, A, B, scalar );
   }
   //**********************************************************************************************

   //**BLAS-based subraction assignment to dense matrices******************************************
#if BLAZE_BLAS_MODE && BLAZE_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION
   /*!\brief BLAS-based subraction assignment of a scaled transpose dense matrix-transpose dense
   //        matrix multiplication (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param C The target left-hand side dense matrix.
   // \param A The left-hand side multiplication operand.
   // \param B The right-hand side multiplication operand.
   // \param scalar The scaling factor.
   // \return void
   //
   // This function performs the scaled transpose dense matrix-transpose dense matrix multiplication
   // based on the according BLAS functionality.
   */
   template< typename MT3    // Type of the left-hand side target matrix
           , typename MT4    // Type of the left-hand side matrix operand
           , typename MT5    // Type of the right-hand side matrix operand
           , typename ST2 >  // Type of the scalar value
   static inline EnableIf_< UseBlasKernel<MT3,MT4,MT5,ST2> >
      selectBlasSubAssignKernel( MT3& C, const MT4& A, const MT5& B, ST2 scalar )
   {
      using ET = ElementType_<MT3>;

      if( IsTriangular<MT4>::value ) {
         ResultType_<MT3> tmp( serial( B ) );
         trmm( tmp, A, CblasLeft, ( IsLower<MT4>::value )?( CblasLower ):( CblasUpper ), ET(scalar) );
         subAssign( C, tmp );
      }
      else if( IsTriangular<MT5>::value ) {
         ResultType_<MT3> tmp( serial( A ) );
         trmm( tmp, B, CblasRight, ( IsLower<MT5>::value )?( CblasLower ):( CblasUpper ), ET(scalar) );
         subAssign( C, tmp );
      }
      else {
         gemm( C, A, B, ET(-scalar), ET(1) );
      }
   }
#endif
   //**********************************************************************************************

   //**Restructuring subtraction assignment to row-major matrices**********************************
   /*!\brief Restructuring subtraction assignment of a scaled transpose dense matrix-transpose
   //        dense matrix multiplication to a row-major matrix (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side scaled multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the symmetry-based restructuring subtraction assignment of a scaled
   // transpose dense matrix-transpose dense matrix multiplication expression to a row-major matrix.
   // Due to the explicit application of the SFINAE principle this function can only be selected
   // by the compiler in case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      subAssign( Matrix<MT,false>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         subAssign( ~lhs, fwd( trans( left ) * trans( right ) ) * rhs.scalar_ );
      else if( IsSymmetric<MT1>::value )
         subAssign( ~lhs, fwd( trans( left ) * right ) * rhs.scalar_ );
      else
         subAssign( ~lhs, fwd( left * trans( right ) ) * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Subtraction assignment to sparse matrices***************************************************
   // No special implementation for the subtraction assignment to sparse matrices.
   //**********************************************************************************************

   //**Schur product assignment to dense matrices**************************************************
   /*!\brief Schur product assignment of a scaled transpose dense matrix-transpose dense matrix
   //        multiplication to a dense matrix (\f$ C\circ=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression for the Schur product.
   // \return void
   //
   // This function implements the performance optimized Schur product assignment of a scaled
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline void schurAssign( DenseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ResultType tmp( serial( rhs ) );
      schurAssign( ~lhs, tmp );
   }
   //**********************************************************************************************

   //**Schur product assignment to sparse matrices*************************************************
   // No special implementation for the Schur product assignment to sparse matrices.
   //**********************************************************************************************

   //**Multiplication assignment to dense matrices*************************************************
   // No special implementation for the multiplication assignment to dense matrices.
   //**********************************************************************************************

   //**Multiplication assignment to sparse matrices************************************************
   // No special implementation for the multiplication assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP assignment to dense matrices************************************************************
   /*!\brief SMP assignment of a scaled transpose dense matrix-transpose dense matrix
   //        multiplication to a dense matrix (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side scaled multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized SMP assignment of a scaled transpose
   // dense matrix-transpose dense matrix multiplication expression to a dense matrix. Due to
   // the explicit application of the SFINAE principle, this function can only be selected by
   // the compiler in case either of the two matrix operands requires an intermediate evaluation
   // and no symmetry can be exploited.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAssign( DenseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL ) {
         return;
      }
      else if( left.columns() == 0UL ) {
         reset( ~lhs );
         return;
      }

      LT A( left  );  // Evaluation of the left-hand side dense matrix operand
      RT B( right );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == left.rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == left.columns()  , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == right.rows()    , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == right.columns() , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns(), "Invalid number of columns" );

      smpAssign( ~lhs, A * B * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**SMP assignment to sparse matrices***********************************************************
   /*!\brief SMP assignment of a scaled transpose dense matrix-transpose dense matrix
   //        multiplication to a sparse matrix (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side sparse matrix.
   // \param rhs The right-hand side scaled multiplication expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized SMP assignment of a scaled transpose
   // dense matrix-transpose dense matrix multiplication expression to a sparse matrix. Due to
   // the explicit application of the SFINAE principle, this function can only be selected by
   // the compiler in case either of the two matrix operands requires an intermediate evaluation
   // and no symmetry can be exploited.
   */
   template< typename MT  // Type of the target sparse matrix
           , bool SO >    // Storage order of the target sparse matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAssign( SparseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      using TmpType = IfTrue_< SO, ResultType, OppositeType >;

      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( OppositeType );
      BLAZE_CONSTRAINT_MATRICES_MUST_HAVE_SAME_STORAGE_ORDER( MT, TmpType );
      BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( TmpType );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      const TmpType tmp( rhs );
      smpAssign( ~lhs, fwd( tmp ) );
   }
   //**********************************************************************************************

   //**Restructuring SMP assignment to row-major matrices******************************************
   /*!\brief Restructuring SMP assignment of a scaled transpose dense matrix-transpose dense matrix
   //        multiplication to a row-major matrix (\f$ C=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side scaled multiplication expression to be assigned.
   // \return void
   //
   // This function implements the symmetry-based restructuring SMP assignment of a scaled
   // transpose dense matrix-dense matrix multiplication expression to a row-major matrix. Due
   // to the explicit application of the SFINAE principle this function can only be selected by
   // the compiler in case the symmetry of either of the two matrix operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      smpAssign( Matrix<MT,false>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         smpAssign( ~lhs, fwd( trans( left ) * trans( right ) ) * rhs.scalar_ );
      else if( IsSymmetric<MT1>::value )
         smpAssign( ~lhs, fwd( trans( left ) * right ) * rhs.scalar_ );
      else
         smpAssign( ~lhs, fwd( left * trans( right ) ) * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**SMP addition assignment to dense matrices***************************************************
   /*!\brief SMP addition assignment of a scaled transpose dense matrix-transpose dense matrix
   //        multiplication to a dense matrix (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be added.
   // \return void
   //
   // This function implements the performance optimized SMP addition assignment of a scaled
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   // Due to the explicit application of the SFINAE principle, this function can only be selected
   // by the compiler in case either of the two matrix operands requires an intermediate evaluation
   // and no symmetry can be exploited.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpAddAssign( DenseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || left.columns() == 0UL ) {
         return;
      }

      LT A( left  );  // Evaluation of the left-hand side dense matrix operand
      RT B( right );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == left.rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == left.columns()  , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == right.rows()    , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == right.columns() , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns(), "Invalid number of columns" );

      smpAddAssign( ~lhs, A * B * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Restructuring SMP addition assignment to row-major matrices*********************************
   /*!\brief Restructuring SMP addition assignment of a scaled transpose dense matrix-transpose
   //        dense matrix multiplication to a row-major matrix (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side scaled multiplication expression to be added.
   // \return void
   //
   // This function implements the symmetry-based restructuring SMP addition assignment of a
   // scaled transpose dense matrix-transpose dense matrix multiplication expression to a
   // row-major matrix. Due to the explicit application of the SFINAE principle this operator
   // can only be selected by the compiler in case the symmetry of either of the two matrix
   // operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      smpAddAssign( Matrix<MT,false>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         smpAddAssign( ~lhs, fwd( trans( left ) * trans( right ) ) * rhs.scalar_ );
      else if( IsSymmetric<MT1>::value )
         smpAddAssign( ~lhs, fwd( trans( left ) * right ) * rhs.scalar_ );
      else
         smpAddAssign( ~lhs, fwd( left * trans( right ) ) * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**SMP addition assignment to sparse matrices**************************************************
   // No special implementation for the SMP addition assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP subtraction assignment to dense matrices************************************************
   /*!\brief SMP subtraction assignment of a scaled transpose dense matrix-transpose dense matrix
   //        multiplication to a dense matrix (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the performance optimized SMP subtraction assignment of a scaled
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   // Due to the explicit application of the SFINAE principle, this function can only be selected
   // by the compiler in case either of the two matrix operands requires an intermediate evaluation
   // and no symmetry can be exploited.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline EnableIf_< IsEvaluationRequired<MT,MT1,MT2> >
      smpSubAssign( DenseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( (~lhs).rows() == 0UL || (~lhs).columns() == 0UL || left.columns() == 0UL ) {
         return;
      }

      LT A( left  );  // Evaluation of the left-hand side dense matrix operand
      RT B( right );  // Evaluation of the right-hand side dense matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == left.rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == left.columns()  , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == right.rows()    , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == right.columns() , "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns(), "Invalid number of columns" );

      smpSubAssign( ~lhs, A * B * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**Restructuring SMP subtraction assignment to row-major matrices******************************
   /*!\brief Restructuring SMP subtraction assignment of a scaled transpose dense matrix-transpose
   //        dense matrix multiplication to a row-major matrix (\f$ C-=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side matrix.
   // \param rhs The right-hand side scaled multiplication expression to be subtracted.
   // \return void
   //
   // This function implements the symmetry-based restructuring SMP subtraction assignment of
   // a scaled transpose dense matrix-transpose dense matrix multiplication expression to a
   // row-major matrix. Due to the explicit application of the SFINAE principle this operator
   // can only be selected by the compiler in case the symmetry of either of the two matrix
   // operands can be exploited.
   */
   template< typename MT >  // Type of the target matrix
   friend inline EnableIf_< CanExploitSymmetry<MT,MT1,MT2> >
      smpSubAssign( Matrix<MT,false>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ForwardFunctor fwd;

      LeftOperand_<MMM>  left ( rhs.matrix_.leftOperand()  );
      RightOperand_<MMM> right( rhs.matrix_.rightOperand() );

      if( IsSymmetric<MT1>::value && IsSymmetric<MT2>::value )
         smpSubAssign( ~lhs, fwd( trans( left ) * trans( right ) ) * rhs.scalar_ );
      else if( IsSymmetric<MT1>::value )
         smpSubAssign( ~lhs, fwd( trans( left ) * right ) * rhs.scalar_ );
      else
         smpSubAssign( ~lhs, fwd( left * trans( right ) ) * rhs.scalar_ );
   }
   //**********************************************************************************************

   //**SMP subtraction assignment to sparse matrices***********************************************
   // No special implementation for the SMP subtraction assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP Schur product assignment to dense matrices**********************************************
   /*!\brief SMP Schur product assignment of a scaled transpose dense matrix-transpose dense
   //        matrix multiplication to a dense matrix (\f$ C+=s*A*B \f$).
   // \ingroup dense_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side multiplication expression for the Schur product.
   // \return void
   //
   // This function implements the performance optimized SMP Schur product assignment of a scaled
   // transpose dense matrix-transpose dense matrix multiplication expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline void smpSchurAssign( DenseMatrix<MT,SO>& lhs, const DMatScalarMultExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( ResultType );
      BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      const ResultType tmp( rhs );
      smpSchurAssign( ~lhs, tmp );
   }
   //**********************************************************************************************

   //**SMP Schur product assignment to sparse matrices*********************************************
   // No special implementation for the SMP Schur product assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP multiplication assignment to dense matrices*********************************************
   // No special implementation for the SMP multiplication assignment to dense matrices.
   //**********************************************************************************************

   //**SMP multiplication assignment to sparse matrices********************************************
   // No special implementation for the SMP multiplication assignment to sparse matrices.
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( MMM );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MMM );
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_BE_NUMERIC_TYPE( ST );
   BLAZE_CONSTRAINT_MUST_BE_SAME_TYPE( ST, RightOperand );
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  GLOBAL BINARY ARITHMETIC OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Multiplication operator for the multiplication of two column-major dense matrices
//        (\f$ A=B*C \f$).
// \ingroup dense_matrix
//
// \param lhs The left-hand side matrix for the multiplication.
// \param rhs The right-hand side matrix for the multiplication.
// \return The resulting matrix.
// \exception std::invalid_argument Matrix sizes do not match.
//
// This operator represents the multiplication of two column-major dense matrices:

   \code
   using blaze::columnMajor;

   blaze::DynamicMatrix<double,columnMajor> A, B, C;
   // ... Resizing and initialization
   C = A * B;
   \endcode

// The operator returns an expression representing a dense matrix of the higher-order element
// type of the two involved matrix element types \a MT1::ElementType and \a MT2::ElementType.
// Both matrix types \a MT1 and \a MT2 as well as the two element types \a MT1::ElementType
// and \a MT2::ElementType have to be supported by the MultTrait class template.\n
// In case the current number of columns of \a lhs and the current number of rows of \a rhs
// don't match, a \a std::invalid_argument is thrown.
*/
template< typename MT1    // Type of the left-hand side dense matrix
        , typename MT2 >  // Type of the right-hand side dense matrix
inline decltype(auto)
   operator*( const DenseMatrix<MT1,true>& lhs, const DenseMatrix<MT2,true>& rhs )
{
   BLAZE_FUNCTION_TRACE;

   if( (~lhs).columns() != (~rhs).rows() ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Matrix sizes do not match" );
   }

   using ReturnType = const TDMatTDMatMultExpr<MT1,MT2,false,false,false,false>;
   return ReturnType( ~lhs, ~rhs );
}
//*************************************************************************************************




//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-symmetric matrix multiplication expression as symmetric.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared dense matrix multiplication expression.
// \exception std::invalid_argument Invalid symmetric matrix specification.
//
// The \a declsym function declares the given non-symmetric matrix multiplication expression
// \a dm as symmetric. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument exception
// is thrown.\n
// The following example demonstrates the use of the \a declsym function:

   \code
   using blaze::columnMajor;

   blaze::DynamicMatrix<double,columnMajor> A, B, C;
   // ... Resizing and initialization
   C = declsym( A * B );
   \endcode
*/
template< typename MT1  // Type of the left-hand side dense matrix
        , typename MT2  // Type of the right-hand side dense matrix
        , bool SF       // Symmetry flag
        , bool HF       // Hermitian flag
        , bool LF       // Lower flag
        , bool UF >     // Upper flag
inline decltype(auto) declsym( const TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid symmetric matrix specification" );
   }

   using ReturnType = const TDMatTDMatMultExpr<MT1,MT2,true,HF,LF,UF>;
   return ReturnType( dm.leftOperand(), dm.rightOperand() );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-Hermitian matrix multiplication expression as Hermitian.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared dense matrix multiplication expression.
// \exception std::invalid_argument Invalid Hermitian matrix specification.
//
// The \a declherm function declares the given non-Hermitian matrix multiplication expression
// \a dm as Hermitian. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument exception
// is thrown.\n
// The following example demonstrates the use of the \a declherm function:

   \code
   using blaze::columnMajor;

   blaze::DynamicMatrix<double,columnMajor> A, B, C;
   // ... Resizing and initialization
   C = declherm( A * B );
   \endcode
*/
template< typename MT1  // Type of the left-hand side dense matrix
        , typename MT2  // Type of the right-hand side dense matrix
        , bool SF       // Symmetry flag
        , bool HF       // Hermitian flag
        , bool LF       // Lower flag
        , bool UF >     // Upper flag
inline decltype(auto) declherm( const TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid Hermitian matrix specification" );
   }

   using ReturnType = const TDMatTDMatMultExpr<MT1,MT2,SF,true,LF,UF>;
   return ReturnType( dm.leftOperand(), dm.rightOperand() );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-lower matrix multiplication expression as lower.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared dense matrix multiplication expression.
// \exception std::invalid_argument Invalid lower matrix specification.
//
// The \a decllow function declares the given non-lower matrix multiplication expression
// \a dm as lower. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument
// exception is thrown.\n
// The following example demonstrates the use of the \a decllow function:

   \code
   using blaze::columnMajor;

   blaze::DynamicMatrix<double,columnMajor> A, B, C;
   // ... Resizing and initialization
   C = decllow( A * B );
   \endcode
*/
template< typename MT1  // Type of the left-hand side dense matrix
        , typename MT2  // Type of the right-hand side dense matrix
        , bool SF       // Symmetry flag
        , bool HF       // Hermitian flag
        , bool LF       // Lower flag
        , bool UF >     // Upper flag
inline decltype(auto) decllow( const TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid lower matrix specification" );
   }

   using ReturnType = const TDMatTDMatMultExpr<MT1,MT2,SF,HF,true,UF>;
   return ReturnType( dm.leftOperand(), dm.rightOperand() );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-upper matrix multiplication expression as upper.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared dense matrix multiplication expression.
// \exception std::invalid_argument Invalid upper matrix specification.
//
// The \a declupp function declares the given non-upper matrix multiplication expression
// \a dm as upper. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument
// exception is thrown.\n
// The following example demonstrates the use of the \a declupp function:

   \code
   using blaze::columnMajor;

   blaze::DynamicMatrix<double,columnMajor> A, B, C;
   // ... Resizing and initialization
   C = declupp( A * B );
   \endcode
*/
template< typename MT1  // Type of the left-hand side dense matrix
        , typename MT2  // Type of the right-hand side dense matrix
        , bool SF       // Symmetry flag
        , bool HF       // Hermitian flag
        , bool LF       // Lower flag
        , bool UF >     // Upper flag
inline decltype(auto) declupp( const TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid upper matrix specification" );
   }

   using ReturnType = const TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,true>;
   return ReturnType( dm.leftOperand(), dm.rightOperand() );
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-diagonal matrix multiplication expression as diagonal.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared dense matrix multiplication expression.
// \exception std::invalid_argument Invalid diagonal matrix specification.
//
// The \a decldiag function declares the given non-diagonal matrix multiplication expression
// \a dm as diagonal. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument exception
// is thrown.\n
// The following example demonstrates the use of the \a decldiag function:

   \code
   using blaze::columnMajor;

   blaze::DynamicMatrix<double,columnMajor> A, B, C;
   // ... Resizing and initialization
   C = decldiag( A * B );
   \endcode
*/
template< typename MT1  // Type of the left-hand side dense matrix
        , typename MT2  // Type of the right-hand side dense matrix
        , bool SF       // Symmetry flag
        , bool HF       // Hermitian flag
        , bool LF       // Lower flag
        , bool UF >     // Upper flag
inline decltype(auto) decldiag( const TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF>& dm )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( dm ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid diagonal matrix specification" );
   }

   using ReturnType = const TDMatTDMatMultExpr<MT1,MT2,SF,HF,true,true>;
   return ReturnType( dm.leftOperand(), dm.rightOperand() );
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
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct Size< TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF>, 0UL >
   : public Size<MT1,0UL>
{};

template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct Size< TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF>, 1UL >
   : public Size<MT2,1UL>
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
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsAligned< TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public And< IsAligned<MT1>, IsAligned<MT2> >
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
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsSymmetric< TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public Or< Bool<SF>
              , And< Bool<HF>
                   , IsBuiltin< ElementType_< TDMatTDMatMultExpr<MT1,MT2,false,true,false,false> > > >
              , And< Bool<LF>, Bool<UF> > >
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
template< typename MT1, typename MT2, bool SF, bool LF, bool UF >
struct IsHermitian< TDMatTDMatMultExpr<MT1,MT2,SF,true,LF,UF> >
   : public TrueType
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsLower< TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public Or< Bool<LF>
              , And< IsLower<MT1>, IsLower<MT2> >
              , And< Or< Bool<SF>, Bool<HF> >
                   , IsUpper<MT1>, IsUpper<MT2> > >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISUNILOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsUniLower< TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public Or< And< IsUniLower<MT1>, IsUniLower<MT2> >
              , And< Or< Bool<SF>, Bool<HF> >
                   , IsUniUpper<MT1>, IsUniUpper<MT2> > >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISSTRICTLYLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsStrictlyLower< TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public Or< And< IsStrictlyLower<MT1>, IsLower<MT2> >
              , And< IsStrictlyLower<MT2>, IsLower<MT1> >
              , And< Or< Bool<SF>, Bool<HF> >
                   , Or< And< IsStrictlyUpper<MT1>, IsUpper<MT2> >
                       , And< IsStrictlyUpper<MT2>, IsUpper<MT1> > > > >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsUpper< TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public Or< Bool<UF>
              , And< IsUpper<MT1>, IsUpper<MT2> >
              , And< Or< Bool<SF>, Bool<HF> >
                   , IsLower<MT1>, IsLower<MT2> > >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISUNIUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsUniUpper< TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public Or< And< IsUniUpper<MT1>, IsUniUpper<MT2> >
              , And< Or< Bool<SF>, Bool<HF> >
                   , IsUniLower<MT1>, IsUniLower<MT2> > >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISSTRICTLYUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF >
struct IsStrictlyUpper< TDMatTDMatMultExpr<MT1,MT2,SF,HF,LF,UF> >
   : public Or< And< IsStrictlyUpper<MT1>, IsUpper<MT2> >
              , And< IsStrictlyUpper<MT2>, IsUpper<MT1> >
              , And< Or< Bool<SF>, Bool<HF> >
                   , Or< And< IsStrictlyLower<MT1>, IsLower<MT2> >
                       , And< IsStrictlyLower<MT2>, IsLower<MT1> > > > >
{};
/*! \endcond */
//*************************************************************************************************

} // namespace blaze

#endif