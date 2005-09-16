<?php include_once("zheader.php"); ?>

<p><img src="hline.png" width="100%" height="1" /></p>

<h1>Name <img src="d_arr.png" /></h1>

<p class="pleft">accdb_shadow - The Shadow back-end</p>

<h1>Description <img src="d_arr.png" /></h1>

<p class="block">Upon each successful call to <tt>b_open()</tt>, the module
reads <tt>backends/../vetc/libaccdb/shadow</tt> (or
<tt>/etc/libaccdb/shadow</tt> if the former is not found) to pull its defaults
from there. That configuration file uses <tt>key=value</tt> pairs to define
something. Following keys are recognized (others will be ignored):</p>

<div class="pleft2">
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
</table>
</div>

<h1>Sysctl interface <img src="d_arr.png" /></h1>

<p class="block">The <tt><b>int b_sysctl(unsigned int, ...)</b></tt> function
of the Shadow back-end supports some more sysctls, mostly used for debugging.
They are:</p>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt>
b_sysctl(0xC000, <b>void *</b>state, <b>const char *</b>new_passwd_file);<br />
b_sysctl(0xC001, <b>void *</b>state, <b>const char *</b>new_shadow_file);<br />
b_sysctl(0xC002, <b>void *</b>state, <b>const char *</b>new_group_file);
    </tt></td>
  </tr>
</table>
</div>

<p class="block">These can be used to alter the names of the DB files at
run-time. The database files currently open will not be changed, so the first
time the new database files are used, is on the next call to <tt>b_open()</tt>.

<h1>Database storage architecture <img src="d_arr.png" /></h1>

<p class="block">Within the Shadow user management, account data is spread over
four files, <tt>/etc/passwd</tt>, <tt>/etc/shadow</tt> and <tt>/etc/group</tt>.
In the past (pre-1990 or so), there were only the first two files, as
confidental data found in today's <tt>/etc/shadow</tt> was stored in
<tt>/etc/passwd</tt> and <tt>gshadow</tt> probably did not exist at all
(gshadow has been obsoleted again in 2004), not even in <tt>group</tt>.</p>

<p class="block"><tt>/etc/passwd</tt> is a text file, comprising zero or more
users, each separated by newline. In each line, fields are separated by a colon
(<tt>:</tt>). The different fields are:</p>

<ul>
  <li>login name</li>
  <li>password (encrypted)</li>
  <li>UID, numerical user id</li>
  <li>GID, numerical id of the user's primary group</li>
  <li>GECOS field (Name, custom data)</li>
  <li>Home directory</li>
  <li>Command interpreter (i.e. <tt>/bin/bash</tt>)</li>
</ul>

<p class="block">In a typical <i>shadow</i> system, the 2nd field is always
replaced by an "<tt>x</tt>", which indicates to look into <tt>/etc/shadow</tt>
for the password instead. Access to the <tt>shadow</tt> file is restricted to
mode <tt>0640(root,shadow)</tt>. Any fields after the command interpreter are
marked as reserved. The ACCDB Shadow back-end preserves them for your
pleasure.</p>

<p class="block">The GECOS field itself can have multiple fields, separated by
comma (<tt>,</tt>). You can freely assign any text to it, though it is normally
used to store the name of the user there. Other details added there are Room
Number, Work Phone, Home Phone and Other. An other document says that the
initial values for <tt>nice</tt>, <tt>umask</tt> and <tt>ulimit</tt> can be set
there, but usually no application will use these fields nowadays. C-Spark for
example stores the XUID in the 2nd GECOS field. If the Command Interpreter
field is empty, it is interpreted as <tt>/bin/sh</tt>, however the empty string
is to be preserved.</p>

<p class="block">The <tt>/etc/shadow</tt> file is also composed of newlines and
colons:</p>

<ul>
  <li>login name</li>
  <li>password</li>
  <li>days since January 01 1970 that password was last changed</li>
  <li>days before password may be changed</li>
  <li>days after which password must be changed</li>
  <li>days before password is to expire that user is warned</li>
  <li>days after password expires that account is disabled</li>
  <li>days since January 01 1970 that account is disabled</li>
</ul>

<h1>Issues <img src="d_arr.png" /></h1>

<p class="block">When two applications write to the Shadow back-end, only the
one who is flushed last wins. If each applications adds one user, say
<tt>test1</tt> and <tt>test2</tt>, respectively, only one of these users will
be present afterwards. This is because <tt>test1</tt> will not be in the memory
image where <tt>test2</tt> resides.</p>

<p class="block">However, writing to the ACCDB is way more rare than reading
from it, so this should hardly be a problem.</p>

<p class="block">To round it up a bit, at least the password files are locked
correctly so that your entries will not get mixed up due to simultaenous access
from multiple processes.</p>

<?php include_once("zfooter.php"); ?>
