#!/usr/bin/perl
#
# Example for web-based services.
#
# As per the manpage of IPC::Open3, this will not work on Win32.
#

use IPC::Open3;
use strict;

print "Username: ";
chomp(my $username = <STDIN>);

$SIG{INT} = sub {
	system qw(stty echo);
	$SIG{INT} = "DEFAULT";
	kill(2, $$);
};
print "Password: ";
system qw(stty -echo);
chomp(my $password = <STDIN>);
system qw(stty echo);

my $err_msg;
if (&authenticate($username, $password, \$err_msg)) {
	print "Authentication successful\n";
} else {
	print "Authentication failure. The error text was:\n", $err_msg;
}

#------------------------------------------------------------------------------
sub authenticate($$)
{
	my($username, $password, $eref) = @_;
	local(*FWRITE, *FERR);
	my($pid, $status);

	#
	# Call the tryauth binary
	#
	if (!defined($pid = open3(\*FWRITE, undef, \*FERR, "tryauth"))) {
		die "Could not spawn process: $!\n";
	}

	print FWRITE $username, "\n", $password, "\n";
	close FWRITE;

	waitpid($pid, 0);
	if (($status = $?) != 0) {
		if (ref($eref) eq "SCALAR") {
			$$eref = join(undef, <FERR>);
		} elsif (ref($eref) eq "ARRAY") {
			@$eref = <FERR>;
		}
	}

	close FERR;
	return $status == 0;
}

#==============================================================================
