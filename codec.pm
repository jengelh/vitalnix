#!/usr/bin/perl
#==============================================================================
# Date scrambling functions
#   by Jan Engelhardt <jengelh at gmx de>, 1999 - 2003
#   -- distributed under the GPL >= v2.0, --
#   -- see doc/GPL-v2.0.txt               --
#   Also includes old "version 1" used in original bv-0.x
#==============================================================================
sub date_scramble {
  if($_[0] =~ /\./so) {
   ($day, $month, $year) = ($_[0] =~ /^(\d\d?)\.(\d\d?)\.(\d{4})/so); }
  elsif($_[0] =~ /\//so) {
   ($month, $day, $year) = ($_[0] =~ /^(\d\d?)\/(\d\d?)\/(\d{4})/so); }
  elsif($_[0] =~ /-/so) {
   ($year, $month, $day) = ($_[0] =~ /^(\d{4})-(\d\d?)-(\d\d?)/so); }
  else { $! = "No delimiter found"; return undef; }
  return sprintf "%04X%02X%02X", $year, $month, $day;
}

sub date_unscramble {
  my($year, $month, $day) =
   (hex(substr($_[0], 0, 4)), hex(substr($_[0], 4, 2)),
   hex(substr($_[0], 6, 2)));
  if($_[0] eq "/") { return sprintf "%02d/%02d/%04d", $month, $day, $year; }
  elsif($_[0] eq "-") {
   return sprintf "%04d-%02d-%02d", $year, $month, $year; }
  else { return sprintf "%02d.%02d.%04d", $day, $month, $year; }
}

sub date1_scramble {
  my($tag, $monat, $jahr) = split(/\./o, shift @_);
  $tag = 26 * $tag + 113;
  $monat = 73 * $monat + 113;
  if($jahr < 50) { $jahr = 100 + $jahr; }
  $jahr = 29 * ($jahr - 75) + 113;
  return $tag.$monat.$jahr;
}

sub date1_unscramble {
  my $code = shift @_;
  my($tag, $monat, $jahr) =
   (substr($code, 0, 3), substr($code, 3, 3), substr($code, 6, 3));
  $tag = ($tag - 113) / 26;
  $monat = ($monat - 113) / 73;
  $jahr = ($jahr - 113) / 29 + 75;
  if($jahr > 100) { $jahr -= 100; }
  $jahr += 1900;
  return join(".", $tag, $monat, $jahr);
}

1;

#==[ End of file ]=============================================================
