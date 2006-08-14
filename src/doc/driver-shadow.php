<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block">The "shadow" driver provides access to the traditional UNIX
file-based user database in <tt>/etc/passwd</tt>, <tt>/etc/shadow</tt> and
<tt>/etc/group</tt>. The additional Vitalnix specific database from
<tt>/etc/vxpasswd</tt> and <tt>/etc/vxshadow</tt> is also supported.

<h1>Configuration file</h1>

<p class="block">The following configuration keys from
<tt>/etc/vitalnix/db_shadow.conf</tt> are recognized:</p>

<table border="1" class="bordered">
  <tr>
    <td class="t1"><tt>PASSWD_DB</tt></td>
    <td class="t1">Path to the passwd file, usually <tt>/etc/passwd</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>SHADOW_DB</tt></td>
    <td class="t2">Path to the shadow file, usually <tt>/etc/shadow</tt></td>
  </tr>
  <tr>
    <td class="t1"><tt>GROUP_DB</tt></td>
    <td class="t1">Path to the group file, usually <tt>/etc/group</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>VXPASSWD_DB</tt></td>
    <td class="t2">Path to the optional Vitalnix world-readable database,
      usually <tt>/etc/vxpasswd</tt></td>
  </tr>
  <tr>
    <td class="t1"><tt>VXSHADOW_DB</tt></td>
    <td class="t1">Path to the optional Vitalnix database, usually
      <tt>/etc/vxshadow</tt></td>
  </tr>
</table>

<p class="block">The Shadow driver also reads
<tt>/etc/vitalnix/autouid.conf</tt> to source UID/GID boundaries for automatic
UID/GID generation.</p>

<h1>Database storage architecture</h1>

<p class="block">Within the Shadow storage mechanism, account data is spread
over three plain-text files, <tt>/etc/passwd</tt>, <tt>/etc/shadow</tt> and
<tt>/etc/group</tt>. In the past (pre-1990 or so), there were only the first
two files, as confidental data found in today's <tt>/etc/shadow</tt> was stored
directly in <tt>/etc/passwd</tt>. (There was also another file,
<tt>gshadow</tt>, but which has rarely been used and was obsoleted in
2004.)</p>

<h2>passwd</h2>

<p class="block"><tt>/etc/passwd</tt> is a text file, comprising zero or more
users, each separated by newline. In each line, fields are separated by a colon
(<tt>:</tt>). The different fields are:</p>

<ul>
  <li>login name</li>
  <li>unused field (formerly password)</li>
  <li>UID, numerical user id</li>
  <li>GID, numerical id of the user's primary group</li>
  <li>real name (and possibly custom data)</li>
  <li>home directory</li>
  <li>command interpreter (shell, e.g. <tt>/bin/bash</tt>)</li>
</ul>

<p class="block">In a typical <i>shadow</i> system, the 2nd field always
contains an "<tt>x</tt>", which indicates to look into <tt>/etc/shadow</tt> for
the password instead. Access to the <tt>shadow</tt> file is restricted to mode
<tt>0640(root,shadow)</tt>. Any fields after the command interpreter are marked
as reserved. The Shadow driver preserves them for you.</p>

<p class="block">The GECOS field itself can have multiple fields, separated by
comma (<tt>,</tt>). You can freely assign any text to it, though it is normally
used to store the name of the user there. Other details added there are Room
Number, Work Phone, Home Phone and Other. An other document says that the
initial values for <tt>nice</tt>, <tt>umask</tt> and <tt>ulimit</tt> can be set
there, but usually no application will use these fields nowadays. If the
Command Interpreter field is empty, it is interpreted as <tt>/bin/sh</tt>,
however, the empty string is preserved.</p>

<p class="block">A file entry from a <tt>passwd</tt> file might look like:</p>

<p class="code"><tt>jengelh:x:1500:100:Jan Engelhardt:/home/jengelh:/bin/bash</tt></p>

<h2>shadow</h2>

<p class="block">The <tt>/etc/shadow</tt> file, which is free to not exist, is
also composed of newlines and colons:</p>

<ul>
  <li>login name</li>
  <li>encrypted password</li>
  <li>days since January 01 1970 that password was last changed</li>
  <li>days before password may be changed</li>
  <li>days after which password must be changed</li>
  <li>days before password is to expire that user is warned</li>
  <li>days after password expires that account is disabled</li>
  <li>days since January 01 1970 that account is disabled</li>
</ul>

<p class="block">A line from a <tt>shadow</tt> file might be:</p>

<p class="code"><tt>jengelh:$2a$05$xWG3SMLJ.dbFtkstUPp4feI1eMm3qoijsEO3YXrSzisuYXKrARrIS:13051:0:10000:0:::</tt></p>

<h2>group</h2>

<p class="block">The group database is also stored in a plain-text file,
<tt>/etc/group</tt>. Its fields are:</p>

<ul>
  <li>group name</li>
  <li>unused field</li>
  <li>numerical group ID</li>
  <li>comma-separated list of members <i>who have membership in this group as
    secondary/supplemental group</i> - this is not their primary group</li>
</ul>

<p class="block">A typical line is (this group has no extra users):</p>

<p class="code"><tt>users:x:100:</tt></p>

<h2>vxpasswd</h2>

<p class="block">This file is currently not defined.</p>

<h2>vxshadow</h2>

<p class="block"><tt>/etc/vxshadow</tt> contains data specific to Vitalnix and
is also not essential for the Shadow PDB driver to function properly. It is
also composed like the other files (i.e. plain-text with newlines and colons);
the fields are:</p>

<ul>
  <li>username</li>
  <li>uuid&nbsp;-- external unique user identifier</li>
  <li>pvgrp&nbsp;-- private group descriptor (userdefined string anyhow)</li>
  <li>day on which account will finally be deleted</li>
</ul>

<h1>Default permissions</h1>

<p class="block">The default permissions for the database files should be as
follows:</p>

<table border="1" class="bordered">
  <tr>
    <td class="t1"><tt>/etc/passwd</tt></td>
    <td class="t1">0644(root,shadow)</td>
  </tr>
  <tr>
    <td class="t2"><tt>/etc/shadow</tt></td>
    <td class="t2">0640(root,shadow)</td>
  </tr>
  <tr>
    <td class="t1"><tt>/etc/group</tt></td>
    <td class="t1">0644(root,shadow)</td>
  </tr>
  <tr>
    <td class="t2"><tt>/etc/vxpasswd</tt></td>
    <td class="t2">0644(root,shadow)</td>
  </tr>
  <tr>
    <td class="t1"><tt>/etc/vxshadow</tt></td>
    <td class="t1">0640(root,shadow)</td>
  </tr>
</table>

<?php include_once("Base-footer.php"); ?>
