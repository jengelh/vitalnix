#!/usr/bin/perl
#==============================================================================
# Standard DAUtool date scrambling v1.x
#   by Jan Engelhardt <jengelh@gmx.de>, 1999 - 2002
#------------------------------------------------------------------------------
# DESCRIPTION
#   This is no real "codec"; these two methods are just used to scramble the
#   birthdate of users, so not everybody can read them easily, only the more
#   intellectual.
# 
# SYNOPSIS
#     use Codec::DAUtool;
#     $scrambled = &Codec::DAUtool::scramble($date);
#     $date = &Codec::DAUtool::unscramble($scrambled, $key);
# 
#   $date may be in format of "T.M.YYYY", "M/T/YYYY" or "YYYY-M-T"
#   $key either be "/", "-" or "." (standard)
#   On failure, scramble returns undef.
# 
# FEATURES
#   v1.21 (July 26 2002)
#     - fixed wrong var name, leading to an incorrest result
#
#   v1.20 (February 22 2002)
#     - added support for other date types ("M/T/YYYY", "YYYY-M-T")
#     - using a regex instead of split, to eleminate non-numbers
# 
#   v1.10 - somewhen back in 2001
#     - fixed: DAUtool itself had a different arg passing
# 
#   v1.00 - somewhen back in 2000
#     - redone the scrambling methods. Teiwes's one was totally
#       over-estimated.
# 
#   v0.xx - somewhen back in 1998
#     - initial release (or such)
# 
# LICENSE
#   Codec::DAUtool starting with v2.2 is distributed under the GPL >= v2.0.
#   Previous versions applied to the same license as DAUtool.
# 
#==============================================================================
package Codec::DAUtool;
$VERSION = "v1.21";

sub scramble {
  if($_[0] =~ /\./so) {
   ($day, $month, $year) = ($_[0] =~ /^(\d\d?)\.(\d\d?)\.(\d{4})/so); }
  elsif($_[0] =~ /\//so) {
   ($month, $day, $year) = ($_[0] =~ /^(\d\d?)\/(\d\d?)\/(\d{4})/so); }
  elsif($_[0] =~ /-/so) {
   ($year, $month, $day) = ($_[0] =~ /^(\d{4})-(\d\d?)-(\d\d?)/so); }
  else { $! = "No delimiter found"; return undef; }

  return sprintf "%04X%02X%02X", $year, $month, $day;
}

sub unscramble {
  my($year, $month, $day) =
   (hex(substr($_[0], 0, 4)), hex(substr($_[0], 4, 2)),
   hex(substr($_[0], 6, 2)));
  if($_[0] eq "/") { return sprintf "%02d/%02d/%04d", $month, $day, $year; }
  elsif($_[0] eq "-") {
   return sprintf "%04d-%02d-%02d", $year, $month, $year; }
  else { return sprintf "%02d.%02d.%04d", $day, $month, $year; }
}

return 1;

#==[ End of file ]=============================================================
