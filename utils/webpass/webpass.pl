#!/usr/bin/suidperl
#==============================================================================
# WebPass
#   by Jan Engelhardt <jengelh@gmx.de>, 2001, 2002
# 
# LIZENZ
#   wie DAUtool (siehe <doc/dautool.html>), also GPL >= v2.0
#==============================================================================
#--< Configuration >--
%OPT = (
  "PROGF" => "webpass",
  "PROGV" => "v1.03",
  "PROGD" => "Freitag, 13. Mai 2002",
  "f_passwd" => "/etc/passwd",
  "f_shadow" => "/etc/shadow",
);

$BANS = <<"EOT";
(root|bin|daemon|lp|news|uucp|games|man|at|postgres|mdom|wwwrun|squid|fax|gnats|adabas|amanda|irc|ftp|firewall|named|fnet|gdm|postfix|oracle|mysql|dpbox|ingres|zope|vscan|wnn|pop|perforce|sapdb|db4web|cyrus|nobody)\@.*?\\..*?\\..*?\\..*?
EOT

#====================================================================[ MAIN ]==
if($ENV{"REQUEST_METHOD"} !~ /^POST$/o) {
  $DATA = join(";", @ARGV);
  $ENV{"CONTENT_LENGTH"} ||= length($DATA);
  $ENV{"REMOTE_ADDR"} ||= "127.0.0.1";
}

# ----------------------------------------
&header();
if(!-e $OPT{"f_passwd"}) {
  print "<p><b><i>".$OPT{"f_passwd"}."</i> gibt es nicht!<br />";
  print "Da ist was faul!</b></p>\n\n";
  $f = 1;
}

# ----------------------------------------
if(!-e $OPT{"f_shadow"}) {
  print "<p><b><i>".$OPT{"f_shadow"}."</i> gibt es nicht!</b></p>\n\n";
  $f = 1;
}

# ----------------------------------------
if($> != 0) { print "<p><b>EUID ist nicht 0!</b></p>\n\n"; $f = 1; }

# ----------------------------------------
if($f < 1) {
  if($ENV{"CONTENT_LENGTH"} == 0) { &action0(); }
  else { &action1(); }
}

# ----------------------------------------
&footer();

#====================================================================[ SUBs ]==
sub action0 { # --------------------------------------------[ Startformular ]--
  &lmsg("connect");

  print <<"EOT";
<p align="center"><b>Passwort ändern?</b></p>

<div align="center">
<form method="POST">
<table border="0" cellpadding="5" cellspacing="0">
  <tr>
    <td valign="top"><b>Benutzername</b></td>
    <td valign="top"><input type="text" name="user"></td>
  </tr>
  <tr>
    <td valign="top"><b>Altes Passwort</b></td>
    <td valign="top"><input type="password" name="oldpass"></td>
  </tr>
  <tr>
    <td valign="top"><b>Neues Passwort</b></td>
    <td valign="top"><input type="password" name="newpass"></td>
  </tr>
  <tr>
    <td valign="top"><b>Wiederholen</b></td>
    <td valign="top"><input type="password" name="newpass2"></td>
  </tr>
  <tr>
    <td align="center" valign="bottom" colspan="2"><input type="submit"
      value="Änderungen senden"></td>
  </tr>
</table>
</form>
</div>
EOT
}

sub action1 { # ---------------------------------------------------[ Ändern ]--
  if($DATA eq "") { read(STDIN, $DATA, $ENV{"CONTENT_LENGTH"}); }
  foreach $_ (split(/;/o, $DATA)) {
    ($name, $wert) = split(/=/o, $_);
    $wert =~ tr/+/ /;
    $wert =~ s/\%([0-9a-f]{2})/chr(hex($1))/egi;
    $QS{$name} = $wert;
  }

  # ----------------------------------------
  if($QS{"user"} eq "") {
    &lmsg("try: no user specified");
    print "<p><b>Kein Benutzer angegeben!</b></p>\n\n";
    return 0;
  }

  # ----------------------------------------
  $banned = &checkbans($QS{"user"}, $ENV{"REMOTE_ADDR"});
  if($banned ne "") {
    &lmsg("try: banned ($banned)");
    print "<p><b>Banned by <i>".$banned."</i></b></p>\n\n";
    return 0;
  }

  # ----------------------------------------
  open(PASSWD, "</etc/passwd");
  foreach $_ (<PASSWD>) {
   if($_ =~ /^$QS{"user"}:/is) { $exist = 1; } }
  close PASSWD;

  if($exist < 1) {
    &lsmg("try: no such user ($QS{"user"})");
    print "<p><b>Kein solcher Benutzer vorhanden!</b></p>\n\n";
    return 0;
  }

  # ----------------------------------------
  if($QS{"newpass"} ne $QS{"newpass2"}) {
    &lmsg("try: new password do not match");
    print "<p><b>Passwörter gleichen sich nicht!</b></p>\n\n";
    return 0;
  }

  # ----------------------------------------
  $result = open(SHADOW, "</etc/shadow");
  if($result < 1) {
    &lmsg("webpass was not able to open </etc/shadow>: ".$!);
    print "<p><b>Konnte <i>/etc/shadow</i> nicht öffnen: ".$!."</b></p>\n\n";
    return 0;
  }

  # ----------------------------------------
  foreach $_ (<SHADOW>) {
    chomp($_);
    ($user, $pass, $rest) = ($_ =~ /^(.*?):(.*?):(.*)/s);

    if($user =~ /^$QS{"user"}$/is) {
      ($salt) = ($pass =~ /^(..)/s);
      $opass = crypt($QS{"oldpass"}, $salt);

      if($pass ne $opass) {
        &lmsg("try: authentification error");
        print "<p><b>Authentifizierungsfehler.</b></p>\n\n";
        return 0;
      }

      $pass = crypt($QS{"newpass"}, $salt);
    }

    push(@SHADOW, join(":", $user, $pass, $rest)."\n");
  }

  # ----------------------------------------
  close SHADOW;
  $result = open(SHADOW, ">/etc/shadow");
  if($result < 1) {
    &lmsg("webpass was not able to reopen </etc/shadow>: ".$!);
    print "<p><b>Konnte <i>/etc/shadow</i> nicht öffnen: ".$!."</b></p>\n\n";
    return 0;
  }

  print SHADOW join("", @SHADOW);
  close SHADOW;
  &lmsg("success: changed");
  print "<p><b>Geändert.</b></p>\n\n";
  return 1;
}

sub checkbans {
  my $V1 = undef;
  foreach $V1 (split(/\n/s, $BANS)) {
    if(join("\@", $QS{"user"}, $ENV{"REMOTE_ADDR"}) =~ /^$V1$/i) {
     return $V1; }
  }

  return undef;
}

sub footer {
  print <<"EOT";
<hr />

<p align="right"><i>$OPT{"PROGF"} $OPT{"PROGV"} ($OPT{"PROGD"})<br>
<small>by Jan Engelhardt &lt;<a
href="mailto:jengelh\@gmx.de">jengelh\@gmx.de</a>&gt;</i></small></p>
</body>
</html>
EOT
}

sub header {
  print <<"EOT";
Content-Type: text/html

<html>
<head>
<title>Web-Interface zum Passwortändern</title>
<style type="text/css">
  body { font: 10pt "Arial", "Verdana", "Helvetica", "Times New Roman"; }
</style>
</head>

<body>

EOT
}

sub lmsg {
  open(LOG, ">/var/log/webpass.log");
  printf LOG "[%s/%s] ", scalar(localtime), $ENV{"REMOTE_ADDR"};
  print LOG join(" ", @_)."\n";
  close LOG;
}

#=====================================================================[ EOF ]==
