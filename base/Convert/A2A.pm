#!/usr/bin/perl
#==============================================================================
# Routine for ANSI <=> ASCII char conversion
#   Copyright (C) Jan Engelhardt <jengelh at gmx de>, 1999 - 2002
#   -- distributed under the GPL >= v2.0 --
#==============================================================================
package Convert::A2A;
require Exporter;
@ISA = qw(Exporter);
@EXPORT_OK = qw(&to_ascii &to_ansi);
$VERSION = 20021011;

sub to_ascii {
  my $s = $_[0];
  if(ref($s) eq "SCALAR") { $s = $$s; }
  $s =~ tr/\x80-\xFF/\xC7\xFC\xE9\xE2\xE4\xE0\xE5\xE7\xEA\xEB\xE8\xEF\xEE\xEC\xC4\xC5\xC9\xB5\xC6\xF4\xF7\xF2\xFB\xF9\xDF\xD6\xDC\xF3\xB7\xD1\x9E\x9F\xFF\xAD\x9B\x9C\xB1\x9D\xBC\x00\xBF\xA9\xA6\xAE\xAA\xED\xBD\xBB\xF8\xF1\xFD\xB3\xB4\xE6\x00\xFA\xB8\xB9\xA7\xAF\xAC\xAB\xBE\xA8\xC0\xC1\xC2\xC3\x8E\x8F\x92\x80\xC8\x90\xCA\xCB\xCC\xCD\xCE\xCF\xD0\xA5\xD2\xD3\xD4\xD5\x99\xD7\xD8\xD9\xDA\xDB\x9A\xDD\xDE\xE1\x85\xA0\x83\xE3\x84\x86\x91\x87\x8A\x82\x88\x89\x8D\xA1\x8C\x8B\xF0\xA4\x95\xA2\x93\xF5\x94\xF6\xB0\x97\xA3\x96\x81\xB2\xFE\x98/;
  if($_[1] eq "LINECNV") { $s =~ s/\x0D\x0A/\x0A/gso; }
  return $s;
}

sub to_ansi {
  my $s = $_[0];
  if(ref($s) eq "SCALAR") { $s = $$s; }
  $s =~ tr/\x80-\xFF/\xC7\xFC\xE9\xE2\xE4\xE0\xE5\xE7\xEA\xEB\xE8\xEF\xEE\xEC\xC4\xC5\xC9\xE6\xC6\xF4\xF6\xF2\xFB\xF9\xFF\xD6\xDC\xA2\xA3\xA5\x9E\x9F\xE1\xED\xF3\xFA\xF1\xD1\xAA\xBA\xBF\xA9\xAC\xBD\xBC\xA1\xAB\xBB\xF8\xA4\xFD\xB3\xB4\x91\x14\x9C\xB8\xB9\x15\xAF\xA6\xAE\xBE\xA8\xC0\xC1\xC2\xC3\x8E\x8F\x92\x80\xC8\x90\xCA\xCB\xCC\xCD\xCE\xCF\xD0\x9D\xD2\xD3\xD4\xD5\x99\xD7\xD8\xD9\xDA\xDB\x9A\xDD\xDE\x98\x85\xDF\x83\xE3\x84\x86\xB5\x87\x8A\x82\x88\x89\x8D\xAD\x8C\x8B\xF0\xB1\x95\x9B\x93\xF5\xF7\x94\xB0\x97\xB7\x96\x81\xB2\xFE\xA0/;
  if($_[1] eq "LINECNV") { $s =~ s/\x0A/\x0D\x0A/gso; }
  return $s;
}

return {"s_name" => "ANSI-ASCII Converter", "s_ver" => $VERSION,
 "s_cat" => "StringOps"};

#==[ Documentation ]===========================================================
=pod

=head1 NAME

Convert::A2A - Convert strings between ANSI and ASCII

=head1 INSTALLATION

Installation of Convert::A2A is pretty easy. Just copy the "Convert/A2A.pm" to
somewhere where Perl can find it. Or put it into
I</usr/lib/perl5/site_perl/5.6.1/Convert/A2A.pm>.

=head1 SYNOPSIS

 use Convert::A2A;
 $out_str = &Convert::A2A::to_ascii($in_str, ?LINECNV?);
 $out_str = &Convert::A2A::to_ansi($in_str, ?LINECNV?);

 use Convert::A2A qw(&to_ansi &to_ascii);
 $out_str = &to_ansi(...);

=head1 DESCRIPTION

Converts a string to "typical" ANSI (ANSI-LF = LINUX/UNIX) or "typical" ASCII
(ASCII-CRLF = DOS/CP437).

This is known as the commands C<dos2unix>, C<recode ibmpc..lat1>
(&B<to_ansi>) and C<unix2dos>, C<recode lat1..ibmpc> (&B<to_ascii>).

I<$src_ascii> and I<$src_ansi> can either be a string or scalar reference. If
you use I<LINECNV>, line terminator conversion (C<"\x0D\x0A" E<lt>=E<gt>
"\x0A">) will also be performed.

Normally, there is not anything exported. If you want to have it exported, pass
qw(WHAT_TO_IMPORT) to B<use> or B<import> yourself.

=cut

#==[ End of file ]=============================================================
