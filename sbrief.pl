#!/usr/bin/perl
#==============================================================================
# Serienbrieferstellung
#   Copyright (C) Cristoph Thiel <cthiel1 at linux01 org>, 2001 - 2003
#   Copyright (C) Jan Engelhardt <jengelh at gmx de>, 2001 - 2003
#   -- distributed under the GPL >= v2.0, --
#   -- see doc/GPL-v2.0.txt               --
#   v1.10 :: 22. Juli 2003
#==============================================================================
$VERSION = "1.082";
use Getopt::Long;
select((select(STDOUT), $| = 1)[0]);
select((select(STDERR), $| = 1)[0]);

($inputf, $outputf, $tpl) = ("-", "-", "sbrief.tex");
&Getopt::Long::Configure(qw(bundling pass_through));
&GetOptions("i|inputf=s" => \$inputf, "o|outputf=s" => \$outputf,
 "t|template=s" => \$tpl, "h" => sub { exec "pod2man $0 | man -l -"; });

if($inputf eq "-") {
 print STDERR "Achtung: Input wird von STDIN gelesen.\n"; }

open(TPL, "<".$tpl) || die sprintf "Konnte %s nicht öffnen: %s\n", $tpl, $!;
$brief = join("", <TPL>);
close TPL;

open(DTR, "<".$inputf) ||
 die sprintf "Konnte <%s> nicht öffnen: %s\n", $inputf, $!;
open(OUT, ">/tmp/dautool_sbrief.tex") ||
 die sprintf "Konnte </tmp/dautool_sbrief.tex> nicht öffnen: %s\n", $!;

print OUT <<"--EOT";
\\documentclass[12pt]{article}

\\usepackage{isolatin1}
\\usepackage{graphics}
\\usepackage{german}

\\begin{document}

--EOT

while(defined(my $l = <DQR>)) {
  ++$count;
  if($count > 2) { print OUT "\\newpage\n"; }
  if($count == 1) { next; }
  chomp($l);
  my @l = split(/:/o, $l);
  print STDERR " ".$l[2];
  $current = $brief;
  $current =~ s/__KLASSE__/$l[0]/go;
  $current =~ s/__REALNAME__/$l[1]/go;
  $current =~ s/__USERNAME__/$l[2]/go;
  $current =~ s/__PASSWORD__/$l[3]/go;
  print OUT $current;
}

print OUT "\\end{document}\n";
close OUT;
close DTR;
system "pdflatex", "/tmp/dautool_sbrief.tex";
print STDERR "Finalized: </tmp/dautool_sbrief.tex>, ".
 "</tmp/dautool_sbrief.pdf>\n";

#=====================================================================[ POD ]==
# To view, run "sbrief -h" or "pod2man sbrief | man -l -"
=pod

=head1 NAME

sbrief - Serienbrieferstellung für die Benutzerverwaltung DAUtool

=head1 AUFRUF

sbrief B<[>-i I<inputfile>B<]> B<[>-t I<templatefile>B<]> B<[>-g I<gid>B<]>

=head1 OPTONEN

=over

=item B<-h>

Zeigt diese Hilfe.

=item B<-i> I<FILE>

Liest aus I<FILE>. Wenn nicht gegeben, dann von STDIN (der Standardeingabe).

=item B<-t> I<FILE>

Vorlage für die Serienbriefe. Standard: I<template.tex>

=item B<-g> I<GID>

Bearbeitet nur Benutzer, deren Native Gruppe I<GID> entspricht. (Erwartet die
Group ID, nicht den Gruppennamen.)

=back

=head1 BESCHREIBUNG

Mit SBrief kann man die (DTR-)Benutzerlisten (die DAUtool ausgibt) in eine
serienbriefartige Form bringen. Damit dieser Serienbrief generisch ist, wird
eine PDF-Datei ausgegeben. Die PDF-Datei wird mit Hilfe von pdflatex(1) aus
einer temporären TeX erstellt. D.h. es wird 1 PDF-Datei mit {Anzahl neuer
Benutzer} Seiten erstellt.

Bevor man B<sbrief> startet, kann man die Vorlage bearbeiten, die sich in
I<sbrief.tex> befindet; wahlweise auch in einer anderen Datei, siehe unten.

Die Vorlage wird in TeX geschrieben (mehr Infos zu TeX:
I<http://www.uni-giessen.de/hrz/tex/cookbook/cookbook.html>).

In der Vorlage können folgende Platzhalter vorkommen:

=over

=item __KLASSE__

wird im Serienbrief mit der Klasse des Benutzers ersetzt

=item __REALNAME__

hier wird der Name eingesetzt (im Format: Max Mustermann)

=item __USERNAME__

von DAUtool generierter Username (mmuster)

=item __PASSWORT__

das Anfangspasswort, das von DAUtool vergeben wurde

=back

=cut

#==[ End of file ]=============================================================
