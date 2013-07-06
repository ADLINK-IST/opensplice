package mpc_debug;

# ************************************************************
# Description   : Scope for declaring useful checkpoints in MPC.
#
#                 This package defines a scope for defining do-nothing
#                 subroutines.  Names should suggest checkpoints in the
#                 execution of MPC, and the body of the function should
#                 be empty.  Calls to these functions can be inserted into
#                 various locations inside MPC source code, and a developer
#                 can set breakpoints on these functions to make it
#                 easier to hone in on checkpoints.
#
#                 If a call is useful, but should only be enabled
#                 during debugging (e.g., it's on a critical path and
#                 could negatively affect performance) then one can
#                 simply comment out the call.
#
#                 For ease in finding calls, please always fully scope
#                 the call, e.g., mpc_debug::chkpnt_blah();
#
# Author        : Chris Cleeland
# Create Date   : 14.Dec.2010
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;

# Checkpoints
#
# Please follow convention and begin each checkpoint name with
# the string "chkpnt_".  Adherence will make it easier for
# another developer to locate all occurrences of checkpoints
# within code using a tool like `grep`.

# Called in Driver's processing of *Creators
sub chkpnt_pre_creator_load { }
sub chkpnt_post_creator_load { }
sub chkpnt_pre_creator_create { }
sub chkpnt_post_creator_create { }

# Called in special 'after' keyword processing
# in ProjectCreator::process_assignment
sub chkpnt_pre_after_keyword_assignment { }
sub chkpnt_post_after_keyword_assignment { }

# Called in Parser::read_file
sub chkpnt_pre_read_file { }
sub chkpnt_post_read_file { }

sub chkpnt_pre_parse_base_project { }
sub chkpnt_post_parse_base_project { }

1;
