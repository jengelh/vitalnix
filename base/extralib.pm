#!/usr/bin/perl
#==============================================================================
#   Copyright (C) Jan Engelhardt <jengelh at gmx de>, 2002
#   -- distributed under the GPL >= v2.0 --
#==============================================================================
package extralib;
require Exporter;
@ISA = qw(Exporter);
@EXPORT_OK = qw(&get_basedir &getcf &getcs &getcs_reset &getgrent &getgrmaxent
 &getpwent &hypot &isolc &isouc &kroot &lcut &pstat);
$VERSION = 20021020;

# You may want to copy &get_basedir() as a section into your BEGIN { }
sub get_basedir {
  my $ln = shift @_;

  if(readlink($ln) ne "") {
    my $bd;
    while(readlink($ln) ne "") {
      my $lf = readlink $ln;
      my $ld = ($lf =~ /^(.*)\/[^\/]+$/s)[0];
      $ln = ((substr($lf, 0, 1) ne "/" && $bd ne "") ? $bd."/" : "").$lf;
      if(substr($ld, 0, 1) eq "/" || length($bd) == 0) { $bd = $ld; }
      else { $bd .= "/".$ld; }
    }

    return $bd;
  } else {
    if(($0 =~ /^(.*)\/[^\/]+$/is)[0] ne "") { return $1; }
    else { return "."; }
  }
}

sub getcf {
  local *FH = shift @_;
  my $rv = undef;
  read(FH, $rv, shift @_);
  return $rv;
}

sub getcs {
  my $rv = undef;
  $rv = substr(${$_[0]}, $getcs_p{$_[0]}, $_[1]);
  $getcs_p{$_[0]} += $_[1];
  return $rv;
}

sub getcs_reset { $getcs_p{shift @_} = shift @_; }

sub getgrent {
  my($g, $fg) = @_;
  local *FG;
  $@ = "";

  $fg ||= "/etc/group";
  if(!open(FG, "<".$fg)) {
    $@ = sprintf "Could not open <%s>: %s", $fg, $!;
    return undef;
  }

  $g = lc $g; if($g eq "") { $g = (split(/ /o, $)))[0]; }
  if(!wantarray) {
    while(defined(my $l = <FG>)) {
      my @lg = split(/:/o, $l);
      if($g =~ /\D/iso) { if($g eq lc $lg[0]) { close FG; return $lg[2]; } }
      elsif($lg[2] == $g) { close FG; return lc $lg[0]; }
    }
  } else {
    while(defined(my $l = <FG>)) {
      if($g !~ /\D/iso) {
        my @lg = split(/:/o, $l);
        if($lg[2] == $g) { $g = lc $lg[0]; }
      }

      my @lg = split(/:/o, $l);
      if($g eq lc $lg[0]) {
        close FG;
        return @lg[0,2,3];
      }
    }
  }

  close FG;
  return undef;
}

sub getgrmaxent {
  my($g, $fg, $fp) = @_;
  local(*FG, *FP);
  $fg ||= "/etc/group";
  $fp ||= "/etc/passwd";
  $@ = "";

  if(!open(FG, "<".$fg)) {
    $@ = sprintf "Could not open <%s>: %s", $fg, $!;
    return undef;
  }

  if(!open(FP, "<".$fp)) {
    $@ = sprintf "Could not open <%s>: %s", $fp, $!;
    close FG;
    return undef;
  }

  $g = lc $g; if($g eq "") { $g = (split(/ /o, $)))[0]; }
  while(defined(my $l = <FG>)) {
    if($g =~ /\D/iso) {
      my @lg = split(/:/o, $l);
      if($g eq lc $lg[0]) { $g = $lg[2]; }
    }

    my @lg = split(/:/o, $l);
    if($g == $lg[2]) {
      my $rv;
      close FG;

      while(defined(my $l = <FP>)) {
        my @lp = split(/:/o, $l);
        if($lp[2] == $g) { $rv .= lc($lp[0]).","; }
      }

      close FP;
      if(substr($rv, -1, 1) eq ",") { $rv = substr($rv, 0, -1); }
      return @lg[0,2,3], $rv;
    }
  }
}

sub getpwent {
  my($u, $fp, $fs) = @_;
  local(*FP, *FS);
  $fp ||= "/etc/passwd";
  $fs ||= "/etc/shadow";

  if(!open(FP, "<".$fp)) {
    $@ = sprintf "Could not open <%s>: %s", $fp, $!;
    return undef;
  }

  if(!open(FS, "<".$fs)) {
    $@ = sprintf "Could not open <%s>: %s", $fs, $!;
    close FP;
    return undef;
  }

  $u = lc $u; if($u eq "") { $u = $>; }
  if(!wantarray) {
    while(defined(my $l = <FP>)) {
      my @lp = split(/:/o, $l);
      if($u =~ /\D/iso) {
       if($u eq lc $lp[0]) { close FP; close FS; return $lp[2]; } }
      elsif($lp[2] == $u) { close FP; close FS; return lc $lp[0]; }
    }
  } else {
    while(defined(my $l = <FP>)) {
      if($u !~ /\D/iso) {
        my @lp = split(/:/o, $l);
        if($lp[2] == $u) { $u = lc $lp[0]; }
      }

      my @lp = split(/:/o, $l);
      if($u eq lc $lp[0]) {
        close FP;

        while(defined(my $l = <FS>)) {
          my @ls = split(/:/o, $l);
          if($u eq lc $ls[0]) {
            close FS;
            return @lp[0,2..5], @ls[1..7];
          }
        }

        close FS;
        return @lp[0,2..5];
      }
    }
  }

  close FP;
  close FS;
  return undef;
}

sub hypot { return sqrt($_[0] ** 2 + $_[1] ** 2); };

sub isolc {
  my $IN = shift @_;
  $IN =~ tr/A-Z\xC0-\xD6\xD8-\xDE/a-z\xE0-\xF6\xF8-\xFE/;
  return $IN;
}

sub isouc {
  my $IN = shift @_;
  $IN =~ tr/a-z\xE0-\xF6\xF8-\xFE/A-Z\xC0-\xD6\xD8-\xDE/;
  return $IN;
}

sub kroot { return $_[0] ** (1 / $_[1]); }

sub lcut {
  my($rv, $length) = @_;
  $rv =~ s/[\x09\x0A\x0D\x20\s]+/ /gso;
  $rv =~ s/[\x00-\x19]/_/gso;
  if($length >= 0 && length($rv) > $length) {
   $rv = substr($rv, 0, $length - 1).">"; }
  return $rv;
}

sub pstat {
  # options field:
  # bit 0 (1) - print \r
  # bit 1 (2) - print \n
  # bit 2 (4) - print \n if $count==$max
  my($count, $max, $opt, $t_start, $t_end) = @_;
  my $rv = sprintf "%s%.0f von %.0f (%.2f%%)",
   ($opt & 1) ? "\r" : "", $count, $max, $count * 100 / $max;

  if($t_start ne "") {
    $rv .= sprintf " [elap: %.0fs, est: %.0fs", $t_end -
     $t_start, ($t_end - $t_start) * $max / $count;
  }

  if($t_end - $t_start > 0) {
   $rv .= sprintf ", ajps: %.2f", $count / ($t_end - $t_start); }

  if($t_start ne "") { $rv .= "]\e[K"; }
  if($opt & 2 || ($count == $max && $opt & 4)) { $rv .= "\n"; }
  return $rv;
}

return 1;

#==[ Documentation ]===========================================================
=pod

=head1 FUNCTIONS

=head2 &getcf()

 $string = &getcf(*FILEHANDLE, $length);

&B<getcf> gets I<$length> chars from I<FILEHANDLE>. Unlike B<read>,
&B<getcf> returns the chars, so storing it in a variable is not needed, thus
you could write things like:

 if(unpack("Z32", &getcf(*FH, 32)) ne "OurSignature") {
  print "This is not one of our files\n"; }

But it takes as much memory to complete the action as B<read>.

=head2 &getcs()

&B<getcs> gets chars from a string:

 $string = &getcs(\$ref_to_string, $length);

But if you fill I<$data> with some other contents and / or want to reset the
position pointer, use this:

 &getcs_reset(\$ref_to_string, $position);

=head2 &getgrent()

 $gid = &getgrent($gname, $ETCGROUP);
 $gname = &getgrent($gid, $ETCGROUP);
 ($gname, $gid, $members) = &getgrent($gid_or_gname, $ETCGROUP);

Does the nearly the same as the getgrent() Perl call, but returns some other
fields. Looksup the LNAME / GID and returns

- the GID if called in scalar context and first argument is numeric

- the GNAME if called in scalar context and first argument contains some
non-numbers

- otherwise returns full info (GNAME, GID and members)

Looksup the group LNAME / GID and returns their name, GID and members in
I</etc/passwd>.

I<$ETCGROUP> can contain a filename which holds the group information.
Defaults to I</etc/group>.

If a file cannot be read, fields from that file do not appear.

=head2 &getgrmaxent()

 ($gname, $gid, $members, $members2) =
  &getgrmaxent($gid_or_gname, $ETCGROUP, $ETCPASSWD);

Looksup the group LNAME / GID and returns their name, GID and members in
I</etc/passwd> and I</etc/passwd>.

I<$ETCGROUP> can contain a filename which holds the group information.
Same for I<$ETCPASSWD>. Defaults to I</etc/group> and I</etc/passwd>.

If a file cannot be read, fields from that file do not appear.

=head2 &getpwent()

 $uid = &getpwent($lname, $ETCPASSWD);
 $lname = &getpwent($uid, $ETCPASSWD);
 ($lname, $uid, $gid, $gcos, $home, $shell, 6x $shadowfields) =
  &getpwent($uid_or_lname, $ETCPASSWD, $ETCSHADOW);

Does nearly the same as the getpwent() Perl call, but returns some other
fields. Looksup the user LNAME / GID and returns their passwd and shadow field.

I<$ETCPASSWD> can contain a filename which holds the group information.
Same for I<$ETCSHADOW>. Defaults to I</etc/passwd> and I</etc/shadow>.

If a file cannot be read, fields from that file do not appear.

=head2 &hypot()

Does exactly the same as the B<hypot>() function from libm.

=head2 &isolc(), &isouc()

Same as Perl's internal B<lc>() and B<uc>() (well, not really...) no locale
support but lowers / uppers everything that seems reasonable in ISO-8859-1. (So
ä gets upped to Ä, not regarding the locale.)

=head2 &kroot()

 &kroot(n, k);

Takes the k'th root of n.

=head2 &pstat()

 print &pstat($count, $max, $options, $t_start, $t_end);

Prints the status of a "task", a particular calculation process. Outputs
things_done, max_things, things_done_in_percent, elapsed_time, estimated_time
(rest) and average_jobs_per_second.

Example:

  $t_start = time();
  for(; $i < $max; ++$i) { # here begins the task or part of it
    ...
    print &pstat($i, $max, 5, $t_start, time());
  }

You can omit I<$t_start> and I<$t_end>.

=cut

#==[ End of file ]=============================================================
