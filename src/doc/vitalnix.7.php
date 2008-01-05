<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vitalnix&nbsp;-- user/group account management software</p>

<h1>Description</h1>

<p class="block">The Vitalnix User Management Suite is a collection of programs
and libraries that implement frontend access to databases storing UNIX user and
group accounts.</p>

<h1>History&nbsp;-- The task</h1>

<p class="block">A tool suite (now evolved to the Vitalnix Suite) was developed
in 1997 at the Otto-Hahn-Gymnasium Göttingen, Germany, to import user
descriptions from a text file dump from the secretary office and make
ready-to-use UNIX user accounts out of it, but taking into account existing
accounts. The program updated the Unix passwd database with changes calculated
from that external text file.</p>

<!-- OHG, MPG, THG, Berufsbildene S. II, Bert-Brecht-S. -->

<p class="block">Database access has been modularized so that LDAP support
would be easily possible; the tool suite has also gained many more programs
that relate to the day-to-day administrative job in computer networks;
predominantly in the educational sector, but as LDAP emerges as a common
technology in any area, this tool suite will be helpful.</p>

<h1>Authors</h1>

<p class="block">The original Perl scripts were created by Eike Teiwes in
1997-1999.</p>

<p class="block">Jan Engelhardt has picked them up in fall 1999 keeping them up
to date (including rewriting the tool suite to get it to new levels) and is the
current maintainer, reachable at jengelh&#64;computergmbh.de.</p>

<p class="block">I would like to thank the following people for their beta
testing, suggestions and contributions over the years: Cordula Petzold,
Cristoph Thiel, Eberhard Mönkeberg.</p>

<h1>Limits of existing software</h1>

<p class="block">A lot of user account maintenance programs I have come across
use NSS ("getpwent") for retrieving information and issue backend-specific
(shadow, LDAP) commands for write operations, such as adding, modifying or
deleting a user or group.</p>

<p class="block">The standard NSS API is neither thread-safe nor multi-use safe
(a single thread, but intertwined invocations of functions). The GNU libc
thread-safe extensions do not cover all functions, for example
<code>setpwent()</code>. Write support is totally absent from NSS&nbsp;-- libc
has a mere <code>putpwent()</code> which requires a stdio file handle, which
of course does not work with other storage methods such as LDAP or MYSQL.</p>

<p class="block">So most programs implement their own write handling, each
coming with its own bulk of bugs, limitations or configuration files (and
Vitalnix is surely no different). Gluing programs together requires a
tremendous amount of time and often you just do not trust some programs when
they ask for information that would normally not be required to perform a given
task.</p>

<h1>Current project status</h1>

<p class="block">Vitalnix provides a library, <i>libvxdb</i>, which
encapsulates away the underlying storage mechanism and provides generalized
methods of basic operations such as "add a user", modify, delete. With regard
to user/group information retrieval, it is mostly equivalent to NSS, but is
designed with thread-safety and multi-use safety in mind from the beginning. I
consider the API and code to be rather clean compared to NSS and pwdutils. </p>

<p class="block">Vitalnix is not the solution to everything. For example, it
does not allow you to add arbitrary attributes to a user account, even if the
underlying storage mechanisms could be adopted to do so. The primary focus on
providing a simple (ideally) API and tools to get a typical essential server
system (e.g. Windows network logon with SMB) working.</p>

<p class="block">The Vitalnix package is considered to be in beta stage. It
works for what we do with it, but it has yet to see widespread usage.</p>

<p class="block">Of course, there is a bit more than just
<i>libvxdb</i>. SAMBA Logon Time Restriction and Print Accounting are
two parts for example, which are not directly related to user
management, but they were nonetheless needed.</p>

<p class="block">Refer to the following subsections and the linked manpages for
further reading.</p>

<h1>Part 1&nbsp;-- Basic setup of Vitalnix</h1>

<p class="block">(To be done)</p>

<h1>Part 2&nbsp;-- Basic user/group information</h1>

<p class="block">This group of tools are mostly equivalent to various system
programs, except that they use the Vitalnix API for interacting with
databases.</p>

<table border="1">
	<tr>
		<td><a href="vxfinger.1.php">vxfinger</a>(1)</td>
		<td>user information lookup</td>
	</tr>
	<tr>
		<td><a href="vxid.1.php">vxid</a>(1)</td>
		<td>print user identity (UID/GID/groups)</td>
	</tr>
</table>

<h1>Part 3&nbsp;-- Basic user/group administration</h1>

<p class="block">These tools are equivalent to <code>/usr/sbin/useradd</code>
and friends. They only operate on the common NSS fields, i.e. do not handle
Vitalnix-specific extensions like the Vitalnix Group Tag or Deferred
Deletion. Use these when working with system accounts</p>

<p><a href="vxuseradd.8.php">vxuseradd</a>(8), <a
href="vxusermod.8.php">vxusermod</a>(8), <a
href="vxuserdel.8.php">vxuserdel</a>(8), <a
href="vxgroupadd.8.php">vxgroupadd</a>(8), <a
href="vxgroupmod.8.php">vxgroupmod</a>(8), <a
href="vxgroupdel.8.php">vxgroupdel</a>(8)</p>

<h1>Part 4&nbsp;-- Auxiliary tools and helpers</h1>

<p class="block">A few helper programs which may be used from other scripts or
programs at your option. They provide a program-based interface to the Vitalnix
C API.</p>

<table border="1">
	<tr>
		<td><a href="vxrandpw.1.php">vxrandpw</a>(1)</td>
		<td>Generating random passwords</td>
	</tr>
	<tr>
		<td><a href="vxtryauth.8.php">vxtryauth</a>(8)</td>
		<td>Interface to the PAM system authentication</td>
	</tr>
</table>

<h1>Part 5&nbsp;-- Tools for user/group synchronization</h1>

<p class="block">Synchronizing parts of the user database with an External Data
Source (EDS, usually a text file) is an essential block of the Vitalnix
Suite.</p>

<table border="1">
	<tr>
		<td><a href="vxckuuid.8.php">vxckuuid</a>(8)</td>
		<td>Finding users whose UUID potentially needs correction prior
		to synchronization</td>
	</tr>
	<tr>
		<td><a href="vxfixuuid.8.php">vxfixuuid</a>(8)</td>
		<td>UUID correction for users</td>
	</tr>
	<tr>
		<td><a href="vxusersync.8.php">vxusersync</a>(8)</td>
		<td>User account synchronization</td>
	</tr>
	<tr>
		<td><a href="vxgroupbld.8.php">vxgroupbld</a>(8)</td>
		<td>Group membership synchronization</td>
	</tr>
	<tr>
		<td><a href="vxpwlfmt.8.php">vxpwlfmt</a>(8)</td>
		<td>Password list prettyprinting&nbsp;-- conversion from
		simple list to magnificient documents</td>
	</tr>
	<tr>
		<td><a href="vxnewuser.8.php">vxnewuser</a>(8)</td>
		<td>Single-user addition with handling for Vitalnix-specific
		fields(!)</td>
	</tr>
	<tr>
		<td><a href="steelmill.8.php">steelmill</a>(8)</td>
		<td>Graphical user interface for <i>vxusersync</i> and
		<i>vxpwlfmt</i> (incomplete)</td>
	</tr>
</table>

<h1>Part 6&nbsp;-- Administrative library code</h1>

<p class="block">(To be done)</p>

<table border="1">
	<tr>
		<td><a href="pam_ihlogon.8.php">pam_ihlogon</a>(8)</td>
		<td>Class-based login time restriction module for the PAM stack
		(Pluggable Authentication Modules)</td>
	</tr>
</table>

<h1>Part 7&nbsp;-- Vitalnix CGI programs</h1>

<p class="block">(To be done)</p>

<table border="1">
	<tr>
		<td><a href="vxcgi_chpasswd.8.php">vxcgi_chpasswd</a>(8)</td>
		<td>Set password</td>
	</tr>
	<tr>
		<td><a href="vxcgi_ntactiv.8.php">vxcgi_ntactiv</a>(8)</td>
		<td>Reset SMBNT password to UNIX password</td>
	</tr>
	<tr>
		<td><a href="vxcgi_vwquota.8.php">vxcgi_vwquota</a>(8)</td>
		<td>Display DiskQuota</td>
	</tr>
</table>

<h1>Part 8&nbsp;-- Print accounting</h1>

<p class="block">(To be done)</p>

<table border="1">
	<tr>
		<td><a href="lpacct_filter.8.php">lpacct_filter</a>(8)</td>
		<td>Vitalnix print job analyzer</td>
	</tr>
	<tr>
		<td><a href="lpacct_scv.8.php">lpacct_scv</a>(8)</td>
		<td>Vitalnix print job confirmator&nbsp;- CUPS backend</td>
	</tr>
	<tr>
		<td><a href="lpacview.8.php">lpacview</a>(8)</td>
		<td>PHP webscript for managing print jobs</td>
	</tr>
</table>

<h1>Part 9&nbsp;-- Database drivers</h1>

<p class="block">(To be done)</p>

<table border="1">
	<tr>
		<td><a href="vxdrv_ldap.7.php">vxdrv_ldap</a>(7)</td>
		<td>LDAP database driver</td>
	</tr>
	<tr>
		<td><a href="vxdrv_mmd.7.php">vxdrv_mmd</a>(7)</td>
		<td>Multiple Module pseudo-driver</td>
	</tr>
	<tr>
		<td><a href="vxdrv_mysql.7.php">vxdrv_mysql</a>(7)</td>
		<td>MYSQL database driver</td>
	</tr>
	<tr>
		<td><a href="vxdrv_shadow.7.php">vxdrv_shadow</a>(7)</td>
		<td>Shadow database driver (<code>/etc/passwd</code>)</td>
	</tr>
</table>

<h1>Part 10&nbsp;-- Vitalnix library API</h1>

<p class="block">(To be done)</p>

<table border="1">
	<tr>
		<td><a href="libvxcgi.3.php">libvxcgi</a>(3)</td>
		<td>Helper functions for CGI programs</td>
	</tr>
	<tr>
		<td><a href="libvxcli.3.php">libvxcli</a>(3)</td>
		<td>Helper functions for CLI-based programs</td>
	</tr>
	<tr>
		<td><a href="libvxcore.3.php">libvxcore</a>(3)</td>
		<td>Really core functions; used by database drivers to register
		with <i>libvxdb</i>.</td>
	</tr>
	<tr>
		<td><a href="libvxdb.3.php">libvxdb</a>(3)</td>
		<td>General Vitalnix API for interaction with user databases</td>
	</tr>
	<tr>
		<td><a href="libvxeds.3.php">libvxeds</a>(3)</td>
		<td>Library for parsing External Data Sources</td>
	</tr>
	<tr>
		<td><a href="libvxmdfmt.3.php">libvxmdfmt</a>(3)</td>
		<td>Library for postformatting password lists (used by vxpwlfmt
		and steelmill)</td>
	</tr>
	<tr>
		<td><a href="libvxmdsync.3.php">libvxmdsync</a>(3)</td>
		<td>Library for handling user synchronization</td>
	</tr>
	<tr>
		<td><a href="libvxutil.3.php">libvxutil</a>(3)</td>
		<td>Various helper functions related to user management</td>
	</tr>
</table>

<h1>Part 11&nbsp;-- Vitalnix internal commands</h1>

<p class="block">Tools for inspecting and debugging Vitalnix and the user
database.</p>

<table border="1">
	<tr>
		<td><a href="vxdbdump.8.php">vxdbdump</a>(8)</td>
		<td>Dump contents of database</td>
	</tr>
	<tr>
		<td><a href="vxdbinfo.8.php">vxdbinfo</a>(8)</td>
		<td>Information about database drivers</td>
	</tr>
</table>

<?php include_once("Base-footer.php"); ?>
