/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef C_QUERYTYPES_H
#define C_QUERYTYPES_H

C_CLASS(c_qExpr);
C_CLASS(c_qField);
C_CLASS(c_qConst);
C_CLASS(c_qType);
C_CLASS(c_qVar);
C_CLASS(c_qFunc);
C_CLASS(c_qRange);
C_CLASS(c_qKey);
C_CLASS(c_qPred);

#define c_qExpr(q)  (C_CAST(q, c_qExpr))
#define c_qField(q) (C_CAST(q, c_qField))
#define c_qConst(q) (C_CAST(q, c_qConst))
#define c_qType(q)  (C_CAST(q, c_qType))
#define c_qVar(q)   (C_CAST(q, c_qVar))
#define c_qFunc(q)  (C_CAST(q, c_qFunc))
#define c_qRange(q) (C_CAST(q, c_qRange))
#define c_qKey(q)   (C_CAST(q, c_qKey))
#define c_qPred(q)  (C_CAST(q, c_qPred))

#endif
