#!/usr/bin/perl
#==============================================================================
# cnvep - Programm zur Konvertierung des GECOS-Datums
#   Copyright (C) Jan Engelhardt <jengelh at gmx de>, 2001 - 2003
#   -- distributed under the GPL >= v2.0, --
#   -- see doc/GPL-v2.0.txt               --
#   v1.06 (24. August 2002)
#
#   Konvertierung der Datumskodierung v1 (bv-0.x) oder v2 (bis einschl.
#   v1.71) in der /etc/passwd ins neue v3 Format.
#==============================================================================
use Getopt::Long;
require "datescram.pm";
select((select(STDOUT), $| = 1)[0]);
select((select(STDERR), $| = 1)[0]);
*unscramble = \&date2_unscramble;
*scramble = \&date_scramble;

($inputf, $outputf, $scangid) = ("/etc/passwd", "-", ".*?");
&Getopt::Long::Configure(qw(bundling pass_through));
&GetOptions("1" => \$format1, "2" => \$format2, "i|inputf=s" => \$inputf,
 "o|outputf=s" => \$outputf, "g|gid=i" => \$scangid);

if($format1 && $format2) {
  print STDERR "Es kann nur eine der beiden Optionen -1 und -2 angegeben ",
   "werden!\n";
  exit 1;
}
if(!$format1 && !$format2) {
  print STDERR "Keine Formatoption (-1,-2) ausgewählt,\n",
   "standardmäßig jetzt -2 angenommen.\n";
}
if($format1) { *unscramble = \&date1_unscramble; }

if($scangid eq "") {
  print STDERR "Keine zu durchsuchende Gruppe angegeben.\n",
   "Alle Gruppen werden durchsucht.\n";
}

open(IN, "<".$inputf) ||
 die sprintf "Konnte %s nicht öffnen: %s\n", $inputf, $!;
@IN = <IN>;
close(IN);

open(OUT, ">".$outputf) ||
 die sprintf "Konnte %s nicht öffnen: %s\n", $outputf, $!;
select((select(IN), $| = 1)[0]);
select((select(OUT), $| = 1)[0]);

foreach my $o (@IN) {
  chomp($o);
  my @o = split(/:/o, $o);
  @gcos = split(/,/o, $o[4], 3);
  if($o[3] =~ /^$scangid$/i && $gcos[1] ne "") {
   $gcos[1] = &scramble(&unscramble($gcos[1])); }
  $o[4] = join(",", @gcos);
  print OUT join(":", @o)."\n";
}

#==[ End of file ]=============================================================
