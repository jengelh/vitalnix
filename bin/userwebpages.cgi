#!/usr/bin/perl

use User::pwent;
use strict;
our $CACHE_FILE = "/home/wwwrun/www-users.cache";

if ($ARGV[0] eq "-c") {
	# Called by a cron entry
	print "Creating cache $CACHE_FILE\n";
	create_cache();
	exit 0;
}

print
	"Content-Type: text/html\n\n",
	"<html>\n",
	"<head>\n",
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n",
	"</head>\n\n",
	"<body>\n\n",
	"<h1>Liste der User-Webseiten</h1>\n\n";

my @users;
my $cache_time = load_cache(\@users);

print scalar(@users) <= 15 ?
	"<p>" : "<p style=\"-moz-column-count: 3; min-height: 6em;\">";

foreach my $u (sort { $a->[0] cmp $b->[0] } @users) {
	print "<a href=\"http://ohg.goe.net/~", $u->[0],
	      "/\" target=\"_blank\">", $u->[1], "</a><br />\n";
}

print
	"</p>\n\n",
	"<p>Cache last refreshed on ", scalar(localtime $cache_time), "</p>\n\n",
	"</body>\n",
	"</html>\n";

#------------------------------------------------------------------------------
sub check_dir
{
	my $dir = shift @_;
	my $found = 0;

	if (!-d $dir || !-x $dir || -e "$dir/.nouserindexing") {
		return 0;
	}

	#
	# Keep the search list in sync with httpd.conf
	#
	foreach my $index (qw(.index.php index.php .index.cgi index.cgi
	    .index.pl index.pl .index.html index.html .index.htm index.htm
	    .index.txt index.txt))
	{
		if (!-e "$dir/$index") {
			next;
		}
		if (!-r "$dir/$index") {
			return 0;
		}
		$found = 1;
	}

	return $found;
}

sub create_cache
{
	my $list = shift @_;
	my $u;

	while (defined($u = getpwent())) {
		if ($u->uid() < 900) {
			next;
		}
		DirLoop: foreach my $d (qw(www public_html)) {
			if (check_dir($u->dir()."/$d")) {
				print $u->name(), "\n";
				push(@$list, [$u->name(), realname($u)]);
				last DirLoop;
			}
		}
	}

	write_cache($list);
	return;
}

sub load_cache
{
	my $list = shift @_;
	local *FH;
	my(@s, $t);

	while (++$t < 3) {
		@s = stat($CACHE_FILE);
		if (!defined($s[0])) {
			sleep 1;
			redo;
		}
	}

	if (!defined($s[0])) {
		print "Cache file not stat'able: $!<br />\n";
		return 0;
	}
	if (!open(FH, "< $CACHE_FILE")) {
		print "Error opening cache file: $!<br />\n";
		return 0;
	}

	while (defined(my $user = <FH>) && defined(my $dir = <FH>)) {
		chomp $user;
		chomp $dir;
		push(@$list, [$user, $dir]);
	}

	close FH;
	return $s[9];
}

sub realname
{
	return (split(/,/, $_[0]->gecos()))[0];
}

sub write_cache
{
	my $list  = shift @_;
	my $tries = 0;
	local *FH;

	print "Writing cache to disk\n";
	unlink $CACHE_FILE;

	if (!open(FH, "> $CACHE_FILE")) {
		print STDERR "Unable to write cache: $!\n";
		return;
	}

	foreach my $user (@$list) {
		print FH join("\n", @$user), "\n";
	}

	close FH;
	return;
}
