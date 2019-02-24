//=================================================================================================
/*!
//  \file blaze/math/TypeTraits.h
//  \brief Header file for all type traits
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

#ifndef _BLAZE_MATH_TYPETRAITS_H_
#define _BLAZE_MATH_TYPETRAITS_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../math/typetraits/HasAdd.h"
#include "../math/typetraits/HasConstDataAccess.h"
#include "../math/typetraits/HasDiv.h"
#include "../math/typetraits/HasMax.h"
#include "../math/typetraits/HasMin.h"
#include "../math/typetraits/HasMult.h"
#include "../math/typetraits/HasMutableDataAccess.h"
#include "../math/typetraits/HasSIMDAbs.h"
#include "../math/typetraits/HasSIMDAcos.h"
#include "../math/typetraits/HasSIMDAcosh.h"
#include "../math/typetraits/HasSIMDAdd.h"
#include "../math/typetraits/HasSIMDAsin.h"
#include "../math/typetraits/HasSIMDAsinh.h"
#include "../math/typetraits/HasSIMDAtan.h"
#include "../math/typetraits/HasSIMDAtan2.h"
#include "../math/typetraits/HasSIMDAtanh.h"
#include "../math/typetraits/HasSIMDCbrt.h"
#include "../math/typetraits/HasSIMDCeil.h"
#include "../math/typetraits/HasSIMDConj.h"
#include "../math/typetraits/HasSIMDCos.h"
#include "../math/typetraits/HasSIMDCosh.h"
#include "../math/typetraits/HasSIMDDiv.h"
#include "../math/typetraits/HasSIMDErf.h"
#include "../math/typetraits/HasSIMDErfc.h"
#include "../math/typetraits/HasSIMDExp.h"
#include "../math/typetraits/HasSIMDExp2.h"
#include "../math/typetraits/HasSIMDExp10.h"
#include "../math/typetraits/HasSIMDFloor.h"
#include "../math/typetraits/HasSIMDHypot.h"
#include "../math/typetraits/HasSIMDInvCbrt.h"
#include "../math/typetraits/HasSIMDInvSqrt.h"
#include "../math/typetraits/HasSIMDLog.h"
#include "../math/typetraits/HasSIMDLog2.h"
#include "../math/typetraits/HasSIMDLog10.h"
#include "../math/typetraits/HasSIMDMax.h"
#include "../math/typetraits/HasSIMDMin.h"
#include "../math/typetraits/HasSIMDMult.h"
#include "../math/typetraits/HasSIMDPow.h"
#include "../math/typetraits/HasSIMDRound.h"
#include "../math/typetraits/HasSIMDSin.h"
#include "../math/typetraits/HasSIMDSinh.h"
#include "../math/typetraits/HasSIMDSqrt.h"
#include "../math/typetraits/HasSIMDSub.h"
#include "../math/typetraits/HasSIMDTan.h"
#include "../math/typetraits/HasSIMDTanh.h"
#include "../math/typetraits/HasSIMDTrunc.h"
#include "../math/typetraits/HasSub.h"
#include "../math/typetraits/HighType.h"
#include "../math/typetraits/IsAdaptor.h"
#include "../math/typetraits/IsAddExpr.h"
#include "../math/typetraits/IsAligned.h"
#include "../math/typetraits/IsBand.h"
#include "../math/typetraits/IsBinaryMapExpr.h"
#include "../math/typetraits/IsBLASCompatible.h"
#include "../math/typetraits/IsColumn.h"
#include "../math/typetraits/IsColumnMajorMatrix.h"
#include "../math/typetraits/IsColumns.h"
#include "../math/typetraits/IsColumnVector.h"
#include "../math/typetraits/IsComputation.h"
#include "../math/typetraits/IsContiguous.h"
#include "../math/typetraits/IsCrossExpr.h"
#include "../math/typetraits/IsCustom.h"
#include "../math/typetraits/IsDeclaration.h"
#include "../math/typetraits/IsDeclDiagExpr.h"
#include "../math/typetraits/IsDeclExpr.h"
#include "../math/typetraits/IsDeclHermExpr.h"
#include "../math/typetraits/IsDeclLowExpr.h"
#include "../math/typetraits/IsDeclSymExpr.h"
#include "../math/typetraits/IsDeclUppExpr.h"
#include "../math/typetraits/IsDenseMatrix.h"
#include "../math/typetraits/IsDenseVector.h"
#include "../math/typetraits/IsDiagonal.h"
#include "../math/typetraits/IsDivExpr.h"
#include "../math/typetraits/IsElements.h"
#include "../math/typetraits/IsEvalExpr.h"
#include "../math/typetraits/IsExpression.h"
#include "../math/typetraits/IsGeneral.h"
#include "../math/typetraits/IsHermitian.h"
#include "../math/typetraits/IsIdentity.h"
#include "../math/typetraits/IsInitializer.h"
#include "../math/typetraits/IsInvertible.h"
#include "../math/typetraits/IsLower.h"
#include "../math/typetraits/IsMatEvalExpr.h"
#include "../math/typetraits/IsMatInvExpr.h"
#include "../math/typetraits/IsMatMapExpr.h"
#include "../math/typetraits/IsMatMatAddExpr.h"
#include "../math/typetraits/IsMatMatMapExpr.h"
#include "../math/typetraits/IsMatMatMultExpr.h"
#include "../math/typetraits/IsMatMatSubExpr.h"
#include "../math/typetraits/IsMatrix.h"
#include "../math/typetraits/IsMatScalarDivExpr.h"
#include "../math/typetraits/IsMatScalarMultExpr.h"
#include "../math/typetraits/IsMatSerialExpr.h"
#include "../math/typetraits/IsMatTransExpr.h"
#include "../math/typetraits/IsMatVecMultExpr.h"
#include "../math/typetraits/IsMultExpr.h"
#include "../math/typetraits/IsNumericMatrix.h"
#include "../math/typetraits/IsNumericVector.h"
#include "../math/typetraits/IsOperation.h"
#include "../math/typetraits/IsOpposedView.h"
#include "../math/typetraits/IsPadded.h"
#include "../math/typetraits/IsProxy.h"
#include "../math/typetraits/IsResizable.h"
#include "../math/typetraits/IsRestricted.h"
#include "../math/typetraits/IsRow.h"
#include "../math/typetraits/IsRowMajorMatrix.h"
#include "../math/typetraits/IsRows.h"
#include "../math/typetraits/IsRowVector.h"
#include "../math/typetraits/IsSchurExpr.h"
#include "../math/typetraits/IsSerialExpr.h"
#include "../math/typetraits/IsShrinkable.h"
#include "../math/typetraits/IsSIMDCombinable.h"
#include "../math/typetraits/IsSIMDEnabled.h"
#include "../math/typetraits/IsSIMDPack.h"
#include "../math/typetraits/IsSMPAssignable.h"
#include "../math/typetraits/IsSparseElement.h"
#include "../math/typetraits/IsSparseMatrix.h"
#include "../math/typetraits/IsSparseVector.h"
#include "../math/typetraits/IsSquare.h"
#include "../math/typetraits/IsStatic.h"
#include "../math/typetraits/IsStrictlyLower.h"
#include "../math/typetraits/IsStrictlyTriangular.h"
#include "../math/typetraits/IsStrictlyUpper.h"
#include "../math/typetraits/IsSubExpr.h"
#include "../math/typetraits/IsSubmatrix.h"
#include "../math/typetraits/IsSubvector.h"
#include "../math/typetraits/IsSymmetric.h"
#include "../math/typetraits/IsTemporary.h"
#include "../math/typetraits/IsTransExpr.h"
#include "../math/typetraits/IsTransformation.h"
#include "../math/typetraits/IsTriangular.h"
#include "../math/typetraits/IsTVecMatMultExpr.h"
#include "../math/typetraits/IsUnaryMapExpr.h"
#include "../math/typetraits/IsUniform.h"
#include "../math/typetraits/IsUniLower.h"
#include "../math/typetraits/IsUniTriangular.h"
#include "../math/typetraits/IsUniUpper.h"
#include "../math/typetraits/IsUpper.h"
#include "../math/typetraits/IsVecEvalExpr.h"
#include "../math/typetraits/IsVecMapExpr.h"
#include "../math/typetraits/IsVecScalarDivExpr.h"
#include "../math/typetraits/IsVecScalarMultExpr.h"
#include "../math/typetraits/IsVecSerialExpr.h"
#include "../math/typetraits/IsVector.h"
#include "../math/typetraits/IsVecTransExpr.h"
#include "../math/typetraits/IsVecTVecMultExpr.h"
#include "../math/typetraits/IsVecVecAddExpr.h"
#include "../math/typetraits/IsVecVecDivExpr.h"
#include "../math/typetraits/IsVecVecMapExpr.h"
#include "../math/typetraits/IsVecVecMultExpr.h"
#include "../math/typetraits/IsVecVecSubExpr.h"
#include "../math/typetraits/IsView.h"
#include "../math/typetraits/LowType.h"
#include "../math/typetraits/RemoveAdaptor.h"
#include "../math/typetraits/RequiresEvaluation.h"
#include "../math/typetraits/Size.h"
#include "../math/typetraits/StorageOrder.h"
#include "../math/typetraits/TransposeFlag.h"
#include "../math/typetraits/UnderlyingBuiltin.h"
#include "../math/typetraits/UnderlyingElement.h"
#include "../math/typetraits/UnderlyingNumeric.h"

#endif
