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
#ifndef OS_DECL_ATTRIBUTES_SAL_H
#define OS_DECL_ATTRIBUTES_SAL_H

#if defined(_MSC_VER)
# include <sal.h>
#endif

#ifndef _Acquires_exclusive_lock_
#define _Acquires_exclusive_lock_(e)
#endif
#ifndef _Acquires_lock_
#define _Acquires_lock_(e)
#endif
#ifndef _Acquires_nonreentrant_lock_
#define _Acquires_nonreentrant_lock_(e)
#endif
#ifndef _Acquires_shared_lock_
#define _Acquires_shared_lock_(e)
#endif
#ifndef _Always_
#define _Always_(a)
#endif
#ifndef _Analysis_assume_
#define _Analysis_assume_
#endif
#ifndef _Analysis_assume_lock_acquired_
#define _Analysis_assume_lock_acquired_(e)
#endif
#ifndef _Analysis_assume_lock_held_
#define _Analysis_assume_lock_held_(e)
#endif
#ifndef _Analysis_assume_lock_not_held_
#define _Analysis_assume_lock_not_held_(e)
#endif
#ifndef _Analysis_assume_lock_released_
#define _Analysis_assume_lock_released_(e)
#endif
#ifndef _Analysis_assume_nullterminated_
#define _Analysis_assume_nullterminated_(s)
#endif
#ifndef _Analysis_assume_same_lock_
#define _Analysis_assume_same_lock_(e)
#endif
#ifndef _Analysis_mode_
#define _Analysis_mode_(m)
#endif
#ifndef _Analysis_noreturn_
#define _Analysis_noreturn_
#endif
#ifndef _Analysis_suppress_lock_checking_
#define _Analysis_suppress_lock_checking_(e)
#endif
#ifndef _At_
#define _At_(t,a)
#endif
#ifndef _At_buffer_
#define _At_buffer_(t,i,c,a)
#endif
#ifndef _Benign_race_begin_
#define _Benign_race_begin_
#endif
#ifndef _Benign_race_end_
#define _Benign_race_end_
#endif
#ifndef _COM_Outptr_
#define _COM_Outptr_
#endif
#ifndef _COM_Outptr_opt_
#define _COM_Outptr_opt_
#endif
#ifndef _COM_Outptr_opt_result_maybenull_
#define _COM_Outptr_opt_result_maybenull_
#endif
#ifndef _COM_Outptr_result_maybenull_
#define _COM_Outptr_result_maybenull_
#endif
#ifndef _Check_return_
#define _Check_return_
#endif
#ifndef _Const_
#define _Const_
#endif
#ifndef _Create_lock_level_
#define _Create_lock_level_(n)
#endif
#ifndef _Deref_
#define _Deref_
#endif
#ifndef _Deref_in_range_
#define _Deref_in_range_(l,h)
#endif
#ifndef _Deref_inout_range_
#define _Deref_inout_range_(l,h)
#endif
#ifndef _Deref_out_range_
#define _Deref_out_range_(l,h)
#endif
#ifndef _Enum_is_bitflag_
#define _Enum_is_bitflag_
#endif
#ifndef _Field_range_
#define _Field_range_(l,h)
#endif
#ifndef _Field_size_
#define _Field_size_(s)
#endif
#ifndef _Field_size_bytes_
#define _Field_size_bytes_(s)
#endif
#ifndef _Field_size_bytes_full_
#define _Field_size_bytes_full_(s)
#endif
#ifndef _Field_size_bytes_full_opt_
#define _Field_size_bytes_full_opt_(s)
#endif
#ifndef _Field_size_bytes_opt_
#define _Field_size_bytes_opt_(s)
#endif
#ifndef _Field_size_bytes_part_
#define _Field_size_bytes_part_(s,c)
#endif
#ifndef _Field_size_bytes_part_opt_
#define _Field_size_bytes_part_opt_(s,c)
#endif
#ifndef _Field_size_full_
#define _Field_size_full_(s)
#endif
#ifndef _Field_size_full_opt_
#define _Field_size_full_opt_(s)
#endif
#ifndef _Field_size_opt_
#define _Field_size_opt_(s)
#endif
#ifndef _Field_size_part_
#define _Field_size_part_(s,c)
#endif
#ifndef _Field_size_part_opt_
#define _Field_size_part_opt_(s,c)
#endif
#ifndef _Function_class_
#define _Function_class_(n)
#endif
#ifndef _Function_ignore_lock_checking_
#define _Function_ignore_lock_checking_(e)
#endif
#ifndef _Group_
#define _Group_(a)
#endif
#ifndef _Guarded_by_
#define _Guarded_by_(e)
#endif
#ifndef _Has_lock_kind_
#define _Has_lock_kind_(n)
#endif
#ifndef _Has_lock_level_
#define _Has_lock_level_(n)
#endif
#ifndef _In_
#define _In_
#endif
#ifndef _In_defensive_
#define _In_defensive_(a)
#endif
#ifndef _In_opt_
#define _In_opt_
#endif
#ifndef _In_opt_z_
#define _In_opt_z_
#endif
#ifndef _In_range_
#define _In_range_(l,h)
#endif
#ifndef _In_reads_
#define _In_reads_(s)
#endif
#ifndef _In_reads_bytes_
#define _In_reads_bytes_(s)
#endif
#ifndef _In_reads_bytes_opt_
#define _In_reads_bytes_opt_(s)
#endif
#ifndef _In_reads_opt_
#define _In_reads_opt_(s)
#endif
#ifndef _In_reads_opt_z_
#define _In_reads_opt_z_(s)
#endif
#ifndef _In_reads_or_z_
#define _In_reads_or_z_(s)
#endif
#ifndef _In_reads_to_ptr_
#define _In_reads_to_ptr_(p)
#endif
#ifndef _In_reads_to_ptr_opt_
#define _In_reads_to_ptr_opt_(p)
#endif
#ifndef _In_reads_to_ptr_opt_z_
#define _In_reads_to_ptr_opt_z_(p)
#endif
#ifndef _In_reads_to_ptr_z_
#define _In_reads_to_ptr_z_(p)
#endif
#ifndef _In_reads_z_
#define _In_reads_z_(s)
#endif
#ifndef _In_z_
#define _In_z_
#endif
#ifndef _Inout_
#define _Inout_
#endif
#ifndef _Inout_defensive_
#define _Inout_defensive_(a)
#endif
#ifndef _Inout_opt_
#define _Inout_opt_
#endif
#ifndef _Inout_opt_z_
#define _Inout_opt_z_
#endif
#ifndef _Inout_updates_
#define _Inout_updates_(s)
#endif
#ifndef _Inout_updates_all_
#define _Inout_updates_all_(s)
#endif
#ifndef _Inout_updates_all_opt_
#define _Inout_updates_all_opt_(s)
#endif
#ifndef _Inout_updates_bytes_
#define _Inout_updates_bytes_(s)
#endif
#ifndef _Inout_updates_bytes_all_
#define _Inout_updates_bytes_all_(s)
#endif
#ifndef _Inout_updates_bytes_all_opt_
#define _Inout_updates_bytes_all_opt_(s)
#endif
#ifndef _Inout_updates_bytes_opt_
#define _Inout_updates_bytes_opt_(s)
#endif
#ifndef _Inout_updates_bytes_to_
#define _Inout_updates_bytes_to_(s,c)
#endif
#ifndef _Inout_updates_bytes_to_opt_
#define _Inout_updates_bytes_to_opt_(s,c)
#endif
#ifndef _Inout_updates_opt_
#define _Inout_updates_opt_(s)
#endif
#ifndef _Inout_updates_opt_z_
#define _Inout_updates_opt_z_(s)
#endif
#ifndef _Inout_updates_to_
#define _Inout_updates_to_(s,c)
#endif
#ifndef _Inout_updates_to_opt_
#define _Inout_updates_to_opt_(s,c)
#endif
#ifndef _Inout_updates_z_
#define _Inout_updates_z_(s)
#endif
#ifndef _Inout_z_
#define _Inout_z_
#endif
#ifndef _Interlocked_
#define _Interlocked_
#endif
#ifndef _Interlocked_operand_
#define _Interlocked_operand_
#endif
#ifndef _Literal_
#define _Literal_
#endif
#ifndef _Lock_level_order_
#define _Lock_level_order_(n1,n2)
#endif
#ifndef _Maybe_raises_SEH_exception_
#define _Maybe_raises_SEH_exception_
#endif
#ifndef _Maybenull_
#define _Maybenull_
#endif
#ifndef _Must_inspect_result_
#define _Must_inspect_result_
#endif
#ifndef _No_competing_thread_
#define _No_competing_thread_
#endif
#ifndef _No_competing_thread_begin_
#define _No_competing_thread_begin_
#endif
#ifndef _No_competing_thread_end_
#define _No_competing_thread_end_
#endif
#ifndef _Notliteral_
#define _Notliteral_
#endif
#ifndef _Notnull_
#define _Notnull_
#endif
#ifndef _Notvalid_
#define _Notvalid_
#endif
#ifndef _NullNull_terminated_
#define _NullNull_terminated_
#endif
#ifndef _Null_
#define _Null_
#endif
#ifndef _Null_terminated_
#define _Null_terminated_
#endif
#ifndef _On_failure_
#define _On_failure_(a)
#endif
#ifndef _Out_
#define _Out_
#endif
#ifndef _Out_defensive_
#define _Out_defensive_(a)
#endif
#ifndef _Out_opt_
#define _Out_opt_
#endif
#ifndef _Out_range_
#define _Out_range_(l,h)
#endif
#ifndef _Out_writes_
#define _Out_writes_(s)
#endif
#ifndef _Out_writes_all_
#define _Out_writes_all_(s)
#endif
#ifndef _Out_writes_all_opt_
#define _Out_writes_all_opt_(s)
#endif
#ifndef _Out_writes_bytes_
#define _Out_writes_bytes_(s)
#endif
#ifndef _Out_writes_bytes_all_
#define _Out_writes_bytes_all_(s)
#endif
#ifndef _Out_writes_bytes_all_opt_
#define _Out_writes_bytes_all_opt_(s)
#endif
#ifndef _Out_writes_bytes_opt_
#define _Out_writes_bytes_opt_(s)
#endif
#ifndef _Out_writes_bytes_to_
#define _Out_writes_bytes_to_(s,c)
#endif
#ifndef _Out_writes_bytes_to_opt_
#define _Out_writes_bytes_to_opt_(s,c)
#endif
#ifndef _Out_writes_opt_
#define _Out_writes_opt_(s)
#endif
#ifndef _Out_writes_opt_z_
#define _Out_writes_opt_z_(s)
#endif
#ifndef _Out_writes_to_
#define _Out_writes_to_(s,c)
#endif
#ifndef _Out_writes_to_opt_
#define _Out_writes_to_opt_(s,c)
#endif
#ifndef _Out_writes_to_ptr_
#define _Out_writes_to_ptr_(p)
#endif
#ifndef _Out_writes_to_ptr_opt_
#define _Out_writes_to_ptr_opt_(p)
#endif
#ifndef _Out_writes_to_ptr_opt_z_
#define _Out_writes_to_ptr_opt_z_(p)
#endif
#ifndef _Out_writes_to_ptr_z_
#define _Out_writes_to_ptr_z_(p)
#endif
#ifndef _Out_writes_z_
#define _Out_writes_z_(s)
#endif
#ifndef _Outptr_
#define _Outptr_
#endif
#ifndef _Outptr_opt_
#define _Outptr_opt_
#endif
#ifndef _Outptr_opt_result_buffer_
#define _Outptr_opt_result_buffer_(s)
#endif
#ifndef _Outptr_opt_result_buffer_maybenull_
#define _Outptr_opt_result_buffer_maybenull_(s)
#endif
#ifndef _Outptr_opt_result_buffer_to_
#define _Outptr_opt_result_buffer_to_(s,c)
#endif
#ifndef _Outptr_opt_result_bytebuffer_
#define _Outptr_opt_result_bytebuffer_(s)
#endif
#ifndef _Outptr_opt_result_bytebuffer_maybenull_
#define _Outptr_opt_result_bytebuffer_maybenull_(s)
#endif
#ifndef _Outptr_opt_result_bytebuffer_to_
#define _Outptr_opt_result_bytebuffer_to_(s,c)
#endif
#ifndef _Outptr_opt_result_maybenull_
#define _Outptr_opt_result_maybenull_
#endif
#ifndef _Outptr_opt_result_maybenull_z_
#define _Outptr_opt_result_maybenull_z_
#endif
#ifndef _Outptr_opt_result_nullonfailure_
#define _Outptr_opt_result_nullonfailure_
#endif
#ifndef _Outptr_opt_result_z_
#define _Outptr_opt_result_z_
#endif
#ifndef _Outptr_result_buffer_
#define _Outptr_result_buffer_(s)
#endif
#ifndef _Outptr_result_buffer_maybenull_
#define _Outptr_result_buffer_maybenull_(s)
#endif
#ifndef _Outptr_result_buffer_to_
#define _Outptr_result_buffer_to_(s,c)
#endif
#ifndef _Outptr_result_bytebuffer_
#define _Outptr_result_bytebuffer_(s)
#endif
#ifndef _Outptr_result_bytebuffer_maybenull_
#define _Outptr_result_bytebuffer_maybenull_(s)
#endif
#ifndef _Outptr_result_bytebuffer_to_
#define _Outptr_result_bytebuffer_to_(s,c)
#endif
#ifndef _Outptr_result_maybenull_
#define _Outptr_result_maybenull_
#endif
#ifndef _Outptr_result_maybenull_z_
#define _Outptr_result_maybenull_z_
#endif
#ifndef _Outptr_result_nullonfailure_
#define _Outptr_result_nullonfailure_
#endif
#ifndef _Outptr_result_z_
#define _Outptr_result_z_
#endif
#ifndef _Outref_
#define _Outref_
#endif
#ifndef _Outref_result_buffer_
#define _Outref_result_buffer_(s)
#endif
#ifndef _Outref_result_buffer_all_
#define _Outref_result_buffer_all_(s)
#endif
#ifndef _Outref_result_buffer_all_maybenull_
#define _Outref_result_buffer_all_maybenull_(s)
#endif
#ifndef _Outref_result_buffer_maybenull_
#define _Outref_result_buffer_maybenull_(s)
#endif
#ifndef _Outref_result_buffer_to_
#define _Outref_result_buffer_to_(s,c)
#endif
#ifndef _Outref_result_buffer_to_maybenull_
#define _Outref_result_buffer_to_maybenull_(s,c)
#endif
#ifndef _Outref_result_bytebuffer_
#define _Outref_result_bytebuffer_(s)
#endif
#ifndef _Outref_result_bytebuffer_all_
#define _Outref_result_bytebuffer_all_(s)
#endif
#ifndef _Outref_result_bytebuffer_all_maybenull_
#define _Outref_result_bytebuffer_all_maybenull_(s)
#endif
#ifndef _Outref_result_bytebuffer_maybenull_
#define _Outref_result_bytebuffer_maybenull_(s)
#endif
#ifndef _Outref_result_bytebuffer_to_
#define _Outref_result_bytebuffer_to_(s,c)
#endif
#ifndef _Outref_result_bytebuffer_to_maybenull_
#define _Outref_result_bytebuffer_to_maybenull_(s,c)
#endif
#ifndef _Outref_result_maybenull_
#define _Outref_result_maybenull_
#endif
#ifndef _Outref_result_nullonfailure_
#define _Outref_result_nullonfailure_
#endif
#ifndef _Points_to_data_
#define _Points_to_data_
#endif
#ifndef _Post_
#define _Post_
#endif
#ifndef _Post_defensive_
#define _Post_defensive_
#endif
#ifndef _Post_equal_to_
#define _Post_equal_to_(e)
#endif
#ifndef _Post_invalid_
#define _Post_invalid_
#endif
#ifndef _Post_ptr_invalid_
#define _Post_ptr_invalid_
#endif
#ifndef _Post_readable_byte_size_
#define _Post_readable_byte_size_
#endif
#ifndef _Post_readable_size_
#define _Post_readable_size_
#endif
#ifndef _Post_same_lock_
#define _Post_same_lock_(e1,e2)
#endif
#ifndef _Post_satisfies_
#define _Post_satisfies_(e)
#endif
#ifndef _Post_valid_
#define _Post_valid_
#endif
#ifndef _Post_writable_byte_size_
#define _Post_writable_byte_size_
#endif
#ifndef _Post_writable_size_
#define _Post_writable_size_
#endif
#ifndef _Post_z_
#define _Post_z_
#endif
#ifndef _Pre_
#define _Pre_
#endif
#ifndef _Pre_defensive_
#define _Pre_defensive_
#endif
#ifndef _Pre_equal_to_
#define _Pre_equal_to_(e)
#endif
#ifndef _Pre_maybenull_
#define _Pre_maybenull_
#endif
#ifndef _Pre_notnull_
#define _Pre_notnull_
#endif
#ifndef _Pre_null_
#define _Pre_null_
#endif
#ifndef _Pre_opt_valid_
#define _Pre_opt_valid_
#endif
#ifndef _Pre_readable_byte_size_
#define _Pre_readable_byte_size_
#endif
#ifndef _Pre_readable_size_
#define _Pre_readable_size_
#endif
#ifndef _Pre_satisfies_
#define _Pre_satisfies_(e)
#endif
#ifndef _Pre_unknown_
#define _Pre_unknown_
#endif
#ifndef _Pre_valid_
#define _Pre_valid_
#endif
#ifndef _Pre_writable_byte_size_
#define _Pre_writable_byte_size_
#endif
#ifndef _Pre_writable_size_
#define _Pre_writable_size_
#endif
#ifndef _Pre_z_
#define _Pre_z_
#endif
#ifndef _Prepost_z_
#define _Prepost_z_
#endif
#ifndef _Printf_format_string_
#define _Printf_format_string_
#endif
#ifndef _Printf_format_string_params_
#define _Printf_format_string_params_(x)
#endif
#ifndef _Raises_SEH_exception_
#define _Raises_SEH_exception_
#endif
#ifndef _Readable_bytes_
#define _Readable_bytes_(s)
#endif
#ifndef _Readable_elements_
#define _Readable_elements_(s)
#endif
#ifndef _Releases_exclusive_lock_
#define _Releases_exclusive_lock_(e)
#endif
#ifndef _Releases_lock_
#define _Releases_lock_(e)
#endif
#ifndef _Releases_nonreentrant_lock_
#define _Releases_nonreentrant_lock_(e)
#endif
#ifndef _Releases_shared_lock_
#define _Releases_shared_lock_(e)
#endif
#ifndef _Requires_exclusive_lock_held_
#define _Requires_exclusive_lock_held_(e)
#endif
#ifndef _Requires_lock_held_
#define _Requires_lock_held_(e)
#endif
#ifndef _Requires_lock_not_held_
#define _Requires_lock_not_held_(e)
#endif
#ifndef _Requires_no_locks_held_
#define _Requires_no_locks_held_
#endif
#ifndef _Requires_shared_lock_held_
#define _Requires_shared_lock_held_(e)
#endif
#ifndef _Reserved_
#define _Reserved_
#endif
#ifndef _Result_nullonfailure_
#define _Result_nullonfailure_
#endif
#ifndef _Result_zeroonfailure_
#define _Result_zeroonfailure_
#endif
#ifndef _Ret_
#define _Ret_
#endif
#ifndef _Ret_maybenull_
#define _Ret_maybenull_
#endif
#ifndef _Ret_maybenull_z_
#define _Ret_maybenull_z_
#endif
#ifndef _Ret_notnull_
#define _Ret_notnull_
#endif
#ifndef _Ret_null_
#define _Ret_null_
#endif
#ifndef _Ret_range_
#define _Ret_range_(l,h)
#endif
#ifndef _Ret_valid_
#define _Ret_valid_
#endif
#ifndef _Ret_writes_
#define _Ret_writes_(s)
#endif
#ifndef _Ret_writes_bytes_
#define _Ret_writes_bytes_(s)
#endif
#ifndef _Ret_writes_bytes_maybenull_
#define _Ret_writes_bytes_maybenull_(s)
#endif
#ifndef _Ret_writes_bytes_to_
#define _Ret_writes_bytes_to_(s,c)
#endif
#ifndef _Ret_writes_bytes_to_maybenull_
#define _Ret_writes_bytes_to_maybenull_(s,c)
#endif
#ifndef _Ret_writes_maybenull_
#define _Ret_writes_maybenull_(s)
#endif
#ifndef _Ret_writes_maybenull_z_
#define _Ret_writes_maybenull_z_(s)
#endif
#ifndef _Ret_writes_to_
#define _Ret_writes_to_(s,c)
#endif
#ifndef _Ret_writes_to_maybenull_
#define _Ret_writes_to_maybenull_(s,c)
#endif
#ifndef _Ret_writes_to_ptr_
#define _Ret_writes_to_ptr_(p)
#endif
#ifndef _Ret_writes_to_ptr_z_
#define _Ret_writes_to_ptr_z_(p)
#endif
#ifndef _Ret_writes_z_
#define _Ret_writes_z_(s)
#endif
#ifndef _Ret_z_
#define _Ret_z_
#endif
#ifndef _Return_type_success_
#define _Return_type_success_(c)
#endif
#ifndef _Scanf_format_string_
#define _Scanf_format_string_
#endif
#ifndef _Scanf_format_string_params_
#define _Scanf_format_string_params_(x)
#endif
#ifndef _Scanf_s_format_string_
#define _Scanf_s_format_string_
#endif
#ifndef _Scanf_s_format_string_params_
#define _Scanf_s_format_string_params_(x)
#endif
#ifndef _Strict_type_match_
#define _Strict_type_match_
#endif
#ifndef _Struct_size_bytes_
#define _Struct_size_bytes_(s)
#endif
#ifndef _Success_
#define _Success_(c)
#endif
#ifndef _Use_decl_annotations_
#define _Use_decl_annotations_
#endif
#ifndef _Valid_
#define _Valid_
#endif
#ifndef _When_
#define _When_(c,a)
#endif
#ifndef _Writable_bytes_
#define _Writable_bytes_(s)
#endif
#ifndef _Writable_elements_
#define _Writable_elements_(s)
#endif
#ifndef _Write_guarded_by_
#define _Write_guarded_by_(e)
#endif

#endif /* OS_DECL_ATTRIBUTES_SAL_H */
