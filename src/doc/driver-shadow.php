<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block">The <i>shadow</i> driver provides access to the traditional
UNIX file-based user database in <code>/etc/passwd</code>,
<code>/etc/shadow</code> and <code>/etc/group</code>. The additional Vitalnix
specific database from <code>/etc/vxpasswd</code> and
<code>/etc/vxshadow</code> is also supported.

<h1>Configuration file</h1>

<p class="block">The following configuration keys from
<code>/etc/vitalnix/db_shadow.conf</code> are recognized:</p>

<table border="1">
  <tr>
    <td class="t1"><code>PASSWD_DB</code></td>
    <td class="t1">Path to the passwd file, usually
      <code>/etc/passwd</code></td>
  </tr>
  <tr>
    <td class="t2"><code>SHADOW_DB</code></td>
    <td class="t2">Path to the shadow file, usually
      <code>/etc/shadow</code></td>
  </tr>
  <tr>
    <td class="t1"><code>GROUP_DB</code></td>
    <td class="t1">Path to the group file, usually <code>/etc/group</code></td>
  </tr>
  <tr>
    <td class="t2"><code>VXPASSWD_DB</code></td>
    <td class="t2">Path to the optional Vitalnix world-readable database,
      usually <code>/etc/vxpasswd</code></td>
  </tr>
  <tr>
    <td class="t1"><code>VXSHADOW_DB</code></td>
    <td class="t1">Path to the optional Vitalnix database, usually
      <code>/etc/vxshadow</code></td>
  </tr>
</table>

<p class="block">The Shadow driver also reads
<code>/etc/vitalnix/autouid.conf</code> to source UID/GID boundaries for
automatic UID/GID generation.</p>

<h1>Database storage architecture</h1>

<p class="block">Within the Shadow storage mechanism, account data is spread
over three plain-text files, <code>/etc/passwd</code>, <code>/etc/shadow</code>
and <code>/etc/group</code>. In the past (pre-1990 or so), there were only the
first two files, as confidental data found in today's <code>/etc/shadow</code>
was stored directly in <code>/etc/passwd</code>. (There was also another file,
<code>gshadow</code>, but which has rarely been used and was obsoleted in
2004.)</p>

<h2>passwd</h2>

<p class="block"><code>/etc/passwd</code> is a text file, comprising zero or
more users, each separated by newline. In each line, fields are separated by a
colon (<code>:</code>). The different fields are:</p>

<ul>
  <li>login name</li>
  <li>unused field (formerly password)</li>
  <li>UID, numerical user id</li>
  <li>GID, numerical id of the user's primary group</li>
  <li>real name (and possibly custom data)</li>
  <li>home directory</li>
  <li>command interpreter (shell, e.g. <code>/bin/bash</code>)</li>
</ul>

<p class="block">In a typical <i>shadow</i> system, the 2nd field always
contains an "<code>x</code>", which indicates to look into
<code>/etc/shadow</code> for the password instead. Access to the
<code>shadow</code> file is restricted to mode <code>0640(root,shadow)</code>.
Any fields after the command interpreter are marked as reserved. The Shadow
driver preserves them for you.</p>

<p class="block">The GECOS field itself can have multiple fields, separated by
comma (<code>,</code>). You can freely assign any text to it, though it is
normally used to store the name of the user there. Other details added there
are Room Number, Work Phone, Home Phone and Other. An other document says that
the initial values for <code>nice</code>, <code>umask</code> and
<code>ulimit</code> can be set there, but usually no application will use these
fields nowadays. If the Command Interpreter field is empty, it is interpreted
as <code>/bin/sh</code>, however, the empty string is preserved.</p>

<p class="block">A file entry from a <code>passwd</code> file might look
like:</p>

<p class="code"><code>jengelh:x:1500:100:Jan
Engelhardt:/home/jengelh:/bin/bash</code></p>

<h2>shadow</h2>

<p class="block">The <code>/etc/shadow</code> file, which is free to not exist,
is also composed of newlines and colons:</p>

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

<p class="block">A line from a <code>shadow</code> file might be:</p>

<p class="code"><code>jengelh:$2a$05$xWG3SMLJ.dbFtkstUPp4feI1eMm3qoijsEO3YXrSzisuYXKrARrIS:13051:0:10000:0:::</code></p>

<h2>group</h2>

<p class="block">The group database is also stored in a plain-text file,
<code>/etc/group</code>. Its fields are:</p>

<ul>
  <li>group name</li>
  <li>unused field</li>
  <li>numerical group ID</li>
  <li>comma-separated list of members <i>who have membership in this group as
    secondary/supplemental group</i> - this is not their primary group</li>
</ul>

<p class="block">A typical line is (this group has no extra users):</p>

<p class="code"><code>users:x:100:</code></p>

<h2>vxpasswd</h2>

<p class="block">This file is currently not defined.</p>

<h2>vxshadow</h2>

<p class="block"><code>/etc/vxshadow</code> contains data specific to Vitalnix
and is also not essential for the Shadow PDB driver to function properly. It is
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

<table border="1">
  <tr>
    <td class="t1"><code>/etc/passwd</code></td>
    <td class="t1">0644(root,shadow)</td>
  </tr>
  <tr>
    <td class="t2"><code>/etc/shadow</code></td>
    <td class="t2">0640(root,shadow)</td>
  </tr>
  <tr>
    <td class="t1"><code>/etc/group</code></td>
    <td class="t1">0644(root,shadow)</td>
  </tr>
  <tr>
    <td class="t2"><code>/etc/vxpasswd</code></td>
    <td class="t2">0644(root,shadow)</td>
  </tr>
  <tr>
    <td class="t1"><code>/etc/vxshadow</code></td>
    <td class="t1">0640(root,shadow)</td>
  </tr>
</table>

<?php include_once("Base-footer.php"); ?>
