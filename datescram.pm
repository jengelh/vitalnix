#!/usr/bin/perl
#==============================================================================
# Date scrambling functions
#   by Jan Engelhardt <jengelh at gmx de>, 1999 - 2003
#   -- distributed under the GPL >= v2.0, --
#   -- see doc/GPL-v2.0.txt               --
#   Also includes old version 1 used in original bv-0.x
#   and version 2 used in up to (including) v1.72
#==============================================================================
use Time::Local;

sub date_scramble { return &date4_scramble(@_); }
sub date_unscramble { return &date4_unscramble(@_); }
sub date4_scramble { return &date2_scramble(@_); }
sub date4_unscramble { return &date2_unscramble(@_); }

sub date3_scramble { # v3
  my($day, $month, $year);
  if($_[0] =~ /\./so) {
   ($day, $month, $year) = ($_[0] =~ /^(\d\d?)\.(\d\d?)\.(\d{2,4})/so); }
  elsif($_[0] =~ /\//so) {
   ($month, $day, $year) = ($_[0] =~ /^(\d\d?)\/(\d\d?)\/(\d{2,4})/so); }
  elsif($_[0] =~ /-/so) {
   ($year, $month, $day) = ($_[0] =~ /^(\d{2,4})-(\d\d?)-(\d\d?)/so); }
  else { return sprintf "%08X", $_[0] + 0; }
  return sprintf "%08X", timelocal(0, 0, 12, $day, $month - 1, $year);
}

sub date3_unscramble {
  my(undef, undef, undef, $day, $month, $year) = localtime hex $_[0];

  my $ty = (localtime())[5];
  my $bp = ($ty + 50) % 100;
  my $nc = $ty - $ty % 100;
  if($bp < 50) { $nc += 100; }
  my $ct = $nc - 100;

  if($year >= 1000) { $year -= 1900; }
  elsif($year < 100 && $year >= 0) { $year += ($year > $bp) ? $ct : $nc; }
  return sprintf "%02d.%02d.%04d", $day, ++$month, $year += 1900;
}

sub date2_scramble {
  if($_[0] =~ /\./so) {
   ($day, $month, $year) = ($_[0] =~ /^(\d\d?)\.(\d\d?)\.(\d{2,4})/so); }
  elsif($_[0] =~ /\//so) {
   ($month, $day, $year) = ($_[0] =~ /^(\d\d?)\/(\d\d?)\/(\d{2,4})/so); }
  elsif($_[0] =~ /-/so) {
   ($year, $month, $day) = ($_[0] =~ /^(\d{2,4})-(\d\d?)-(\d\d?)/so); }
  else { $! = "No delimiter found"; return undef; }

  my $ty = (localtime())[5];
  my $bp = ($ty + 50) % 100;
  my $nc = $ty - $ty % 100;
  if($bp < 50) { $nc += 100; }
  my $ct = $nc - 100;

  if($year >= 1000) { $year -= 1900; }
  elsif($year < 100 && $year >= 0) { $year += ($year > $bp) ? $ct : $nc; }
  $year += 1900;
  return sprintf "%04X%02X%02X", $year, $month, $day;
}

sub date2_unscramble {
  return sprintf "%02d.%02d.%04d", hex(substr($_[0], 6, 2)),
   hex(substr($_[0], 4, 2)), hex(substr($_[0], 0, 4));
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
  my($tag, $monat, $jahr) = ($code =~ /^(\d\d\d)(\d\d\d)(\d\d\d)/s);
  $tag = ($tag - 113) / 26;
  $monat = ($monat - 113) / 73;
  $jahr = ($jahr - 113) / 29 + 75;
  if($jahr > 100) { $jahr -= 100; }
  $jahr += 1900;
  return sprintf "%02d.%02d.%04d", $tag, $monat, $jahr;
}

1;

#==[ End of file ]=============================================================
