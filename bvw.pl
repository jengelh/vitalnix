#!/usr/bin/perl -w
#
#	Benutzerverwaltung
#
# gemeinsam genutze Routinen


#
#	Initialize
#
# set some variables
#
sub Initialize {
    local ($domain, $usergroup, $usershell, $staffgroup, $quotausr);
    local ($dflt_filename, $dflt_delimiter);

    $domain = DomainErmitteln();
    $usergroup = 'train';
    $usershell = '/usr/bin/passwd';
    $staffgroup = 'staff';
    $quotausr = 'uhdb';
    $dflt_filename = 'daten.sdf';
    $dflt_delimiter = ';';

    return ($domain, $usergroup, $usershell, $staffgroup, $quotausr,
	    $dflt_filename, $dflt_delimiter);
}


#
#	chown_by_name
#
#
sub chown_by_name {
    local ($user, $pattern) = @_;
    chown( (getpwnam($user))[2,3], glob($pattern) );
}


#
#	DomainErmitteln
#
# Ermitteln des vollstaendigen Domain-Namens
#
# Autor: Markus Boie
#
sub DomainErmitteln {
    local $domain = `/bin/hostname --domain`;
    return $domain;
}


# --------------------------------------------------------------------

#
#	passwd_auslesen2
#
sub passwd_auslesen2 {

    local ($i, $name, $passwd, $uid, $gid, $gcos, $fullname, $code);
    local (%feld);

    $i = 0;
    open(PWD, "/etc/passwd");
    while (<PWD>) {
        chop;
        ($name, $passwd, $uid, $gid, $gcos) = split(/:/);
        #($fullname, $code) = split(/, /, $gcos);
        $feld{$name} = $gcos;
        if ($gid eq $gruppe) {
            ++$i;
        }
    }
    close(PWD);
}

# --------------------------------------------------------------------



1;	return TRUE

#
# end bvw.pl
