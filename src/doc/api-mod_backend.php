<?php include_once("Base-header.php"); ?>

<h1>Module definition</h1>

<p class="block">Each module must define a structure <code>struct
vxpdb_driver</code> in which they set the function pointers to the respective
functions. The module structure also includes space for name, author and
description of the module for display with the <i>pdbinfo</i> utility. A
reduced example definition could look like this:</p>

<p class="code"><code><b>#</b>include "drivers/static-build.h"<br />
<b>#</b>include &lt;vitalnix/libvxpdb/libvxpdb.h&gt;<br />
<br />
<b>static struct</b> vxpdb_driver THIS_MODULE <b>=</b> {<br />
&nbsp; &nbsp; .name &nbsp; &nbsp; &nbsp;<b>=</b> "Our sample module",<br />
&nbsp; &nbsp; .userinfo &nbsp;<b>=</b> our_userinfo,<br />
&nbsp; &nbsp; .groupinfo <b>=</b> our_groupinfo,<br />
};<br />
<br />
REGISTER_MODULE(our, <b>&amp;</b>THIS_MODULE);</code></p>

<p class="block">The <code>drivers/static-build.h</code> include, the
<code>REGISTER_MODULE</code> macro expands to extra code required for
initializion.</p>

<p class="block">Then of course, the module needs to provide the functions we
have just specified in the sturct. They can then be called from user
applications using the <code>vxpdb_*()</code> functions and the respective
instance as obtained from <code>vxpdb_load()</code>. Note that the struct must
be writable since it will be modified.</p>

<!--
<h1>Implementable functions</h1>

<h2>-&gt;init</h2>

<p class="code"><code><b>int (*</b>init<b>)</b>(<b>struct</b> vxpdb_state <b>*</b>mip, const char <b>*</b>config_file);</code></p>

<p class="block"><code>our_init()</code> function gets called after the shared
library has been opened through <code>vxpdb_load()</code> from the caller
program. It gets passed the <code>priv</code> argument from
<code>vxpdb_load()</code>, which may contain module-specific initialization
data. Most of the times however, the caller will just put <code>NULL</code> for
<code>private_data</code>.</p>

<p class="block">In this function, the module should allocate its state and
read configuration files. The allocation of a state might look like below, and
is necessary for reentrancy:</p>

<p class="code"><code><b>static int</b> our_init(<b>struct</b> vxpdb_state <b>*</b>mip, <b>void *</b>priv) {<br />
    <b>struct</b> our_state <b>*</b>state;<br />
    state <b>=</b> mip-&gt;state <b>=</b> malloc(sizeof(struct our_state));<br />
    state-&gt;config <b>=</b> read_some_config("bla.conf");<br />
    read_some_extras(priv);<br />
    return 1;<br />
}</code></p>

<p class="block">This is a very basic example, and you should look at the
already existing modules to see what they do, and possibly how they do it.</p>

<h2>-&gt;open</h2>

<p class="code"><code><b>int (*</b>open<b>)</b>(<b>struct</b> vxpdb_state <b>*</b>mip, <b>long</b> flags);</code></p>

<p class="block">The open function should open a connection to the password
database (if applicable), or do whatever is equivalent to prepare further
actions. The <code>flags</code> parameter is explained in
<code>api-libvxpdb</code>. Some sample code:</p>

<p class="code"><code><b>int</b> our_open(<b>struct</b> vxpdb_state <b>*</b>mip, <b>long</b> flags) {<br />
    <b>struct</b> our_state <b>*</b>state <b>=</b> mip-&gt;state;<br />
    if((state-&gt;fp <b>=</b> fopen("/etc/passwd", "r")) <b>==</b> NULL)<br />
        return -errno;<br />
    return 1;<br />
}</code></p>

<h2>-&gt;close</h2>

<p class="code"><code><b>void (*</b>close<b>)</b>(<b>struct</b> vxpdb_state <b>*</b>mip);</code></p>

<h2>-&gt;exit</h2>

<p class="code"><code><b>void (*</b>exit<b>)</b>(<b>struct</b> vxpdb_state <b>*</b>mip);</code></p>

<h2>-&gt;modctl</h2>

<p class="code"><code><b>long (*</b>modctl<b>)</b>(<b>struct</b> vxpdb_state <b>*</b>mip, <b>long</b> command, <b>...</b>);</code></p>

<h2>-&gt;lock</h2>

<p class="code"><code><b>int (*</b>lock<b>)</b>(<b>struct</b> vxpdb_state <b>*</b>mip);</code></p>

<p class="block">

<h2>-&gt;unlock</h2>

<p class="code"><code><b>int (*</b>unlock<b>)</b>(<b>struct</b> vxpdb_state <b>*</b>mip);</code></p>

<h1></h1>

<h1></h1>

<h1></h1>

<h1></h1>

<h1>Description</h1>

<p class="block">libvxpdb Since there is a great variety of user databases, the
Unified Account Database provides a generic API to any application. Some user
databases are for example the Shadow Password System (<code>/etc/passwd</code>
and friends). Another could be the Samba userdb in <code>/var/lib/samba</code>,
or OpenLDAP (libldap).</p>

-->

<h1>Notes</h1>

<p class="block">Depending on how the user database is organized, you might
want, or even need to read it into memory (=&nbsp;cache it). Be sure to keep a
lock on the database and/or files when writing, if necessary, to not run into
race conditions. Unprivileged access that cannot write does not need any write
locking. Doing the I/O on our own (and even caching if the module author wants
to) can result in a faster execution, and allows for way more control. "Pipe
modules", which just transfer data, as it would be with a LDAP or MySQL
database, do not need to do any direct file I/O, since the underlying DB will
handle that.</p>

<p class="block">It is not specified whether and how the back-end module and/or
the user database can handle accounts with non-unique UIDs (e.g. to implement
an old-style multi-superuser system). However, it should be handled gracefully,
i.e. either allow it in a manageable way or return an appropriate error
code.</p>

<p class="block">The module may or may not allow opening the user database
twice (the module may be opened by different applications). You should handle
that case then. One, locking files, and two, checking for the lock is the usual
thing for a module to be opened only once at a time.</p>

<!-- <p class="block">Upon success, return <code>&gt;0</code>, otherwise return
<code>0</code> (and possibly set <code>errno</code>), or even return
<code>-errno</code> and set <code>errno</code> to signalize a hard error (i.e.
<code>ENOMEM</code> due to <code>malloc()</code>).</p>

<p class="block"><code>b_close()</code> shall deinitialize the currently open
session for the user database. Flush any data to disk if needed and free up all
memory used for the session.</p>

<p class="block"><code>exit()</code> is the counterpart of
<code>b_init()</code> and is called through <code>accdb_unload()</code>. This
function also does not necessarily need to exist either.</p>

<h1>Traversing the user and group lists</h1>

<p class="code"><code><b>int</b> pdb_usertrav(<b>void *</b>state, <b>struct</b> vxpdb_user <b>*</b>result);<br />
<b>int</b> pdb_grouptrav(<b>void *</b>state, <b>struct</b> vxpdb_group <b>*</b>result);</code></p>

<p class="block">The <code>accdb_[ug]entry <b>struct</b></code>s are as
follows:</p>

<p class="code"><code><b>struct</b> vxpdb_user {<br />
&nbsp; &nbsp; // passwd part<br />
&nbsp; &nbsp; <b>char *</b>lname;<br />
&nbsp; &nbsp; <b>long</b> uid, gid;<br />
&nbsp; &nbsp; <b>char *</b>gecos, <b>*</b>home, <b>*</b>shell, <b>*</b>igrp, <b>*</b>sgrp;<br />
<br />
&nbsp; &nbsp; // shadow part<br />
&nbsp; &nbsp; <b>char *</b>pass, <b>*</b>pass_cryp;<br />
&nbsp; &nbsp; <b>long</b> last_change, keep_min, keep_max, warn_age, expire, inactive;<br />
};<br />
<br />
<b>struct</b> vxpdb_group {<br />
&nbsp; &nbsp; <b>long</b> gid;<br />
&nbsp; &nbsp; <b>char *</b>gname;<br />
};</code></p>

<p class="block">Analogous to <code>getpwent()</code>, just a bit more
organized, is the <code>b_usertrav()</code> function. It takes the usual state
pointer and a <code><b>struct</b> vxpdb_user <b>*</b></code> pointer. When
calling <code>b_usertrav()</code>, it takes the next user found in the database
and fills in the struct.</p>

<p class="block">All the <code><b>char *</b></code> fields in the structs shall
point to allocated memory (or memory available throughout the program), so do
not use local variables or <code>alloca()</code>. <code>strdup()</code> or
similar might help you. Those strings may not be changed by anything else than
the driver module, they are meant to be read-only. That way, these strings can
be reused anywhere.</p>

<p class="block">For each successful returned user via
<code>b_usertrav()</code>, the function shall return <code>&gt;0</code>
(usually <code>1</code> suffices). The order of users is of no importance; if
necessary, the application itself will need to sort it. If the end of the list
is reached, return <code>0</code>, or, if an error occurred, set
<code>errno</code> and return <code>-errno</code>. When the end of the list is
reached, <code>b_usertrav()</code> can be called with the <code>result</code>
parameter set to <code>NULL</code> to reset the list traverser.</p>

<p class="block">Similar applies to the group traversing function
<code>b_grouptrav()</code>.</p>

<p class="block">The traversion pointer is not static but stored in the
state.</p>

<h1>Retrieve info about a user or group</h1>

<div class="pleft2">
<table>
  <tr>
    <td class="code"><code><b>int</b> pdb_userinfo(<b>void *</b>state, <b>struct</b> vxpdb_user <b>*</b>req, <b>struct</b> vxpdb_user <b>*</b>dest, <b>size_t</b> s);<br />
<b>int</b> pdb_groupinfo(<b>void *</b>state, <b>struct</b> vxpdb_group <b>*</b>req, <b>struct</b> vxpdb_group <b>*</b>dest, <b>size_t</b> s);<br /></code></td>
  </tr>
</table>
</div>

<p class="block"><code>b_userinfo()</code> walks down the list of users
searching for matching users. The contents of <code>req</code> are compared
with every user in the list. If a value in <code>req</code> is <code>-1</code>
or <code>NULL</code> (depending on the member type and meaning), it is ignored
and not included in the comparison, so you could do i.e. the following
searches:</p>

<p class="pleft2">any user with UID 37007 (<code>req<b>-&gt;</b>lname <b>==
NULL &amp;&amp;</b> req<b>-&gt;</b>uid <b>==</b> 37007</code>),<br />

user "jengelh", any UID (<code>strcmp(req<b>-&gt;</b>lname, "jengelh") == 0
<b>&amp;&amp;</b> req<b>-&gt;</b>uid <b>==</b> -1</code>)<br />

user "jengelh" with UID 37007 (<code>strcmp(req<b>-&gt;</b>lname, "jengelh")
<b>==</b> 0 <b>&amp;&amp;</b> req<b>-&gt;</b>uid <b>==</b> 37007</code>)</p>

<p class="block"><code>b_userinfo()</code> has three different operation cases.
The first is if <code>dest</code> is not <code>NULL</code>, in which case at
most <code>s</code> users are placed into <code>dest</code>, and the number of
users stored (can never be <code>&gt;s</code>, but may be less) is returned.
Case two is that <code>dest</code> is <code>NULL</code> and <code>s</code> is
0, where <code>1</code> is returned as soon as the first match is found. Case
three, <code>dest</code> is <code>NULL</code> and <code>s</code> is 1, no users
are copied, but the number of matches will be returned.</p>

<p class="block">Again, the complement <code>b_groupinfo()</code> behaves just
as like.</p>

<p class="block"><code>b_userinfo()</code> and <code>b_groupinfo()</code> do
not interrupt the traversion with <code>b_usertrav()</code> /
<code>b_grouptrav()</code>. They use their own (local) traversion pointer.</p>

<h1>Adding a user or group</h1>

<div class="pleft2">
<table>
  <tr>
    <td class="code"><code><b>int</b> pdb_useradd(<b>void *</b>state, <b>struct</b> vxpdb_user <b>*</b>user);<br />
<b>int</b> pdb_groupadd(<b>void *</b>state, <b>struct</b> vxpdb_group <b>*</b>group);<br /></code></td>
  </tr>
</table>
</div>

<p class="block"><code>b_useradd</code> creates a new user in the database from
the data provided in <code>user</code>. It duplicates all strings provided by
an application and adds them to the database.</p>

<p class="block">If <code>.lname</code> is <code>NULL</code>, or there is no
group specification in either <code>.gid</code> or <code>.igrp</code>,
<code>errno</code> is to be set to <code>EINVAL</code> and <code>0</code> is to
be returned. For <code>b_groupadd()</code>, <code>errno</code> is set to
<code>EINVAL</code> and <code>0</code> is returned if <code>.gname</code> is
<code>NULL</code>. <code>-errno</code> is returned when something underlying
<code>b_useradd()</code> (or <code>b_groupadd()</code>) failed, like writing to
the database.</p>

<p class="block">If the <code>.uid</code> or <code>.gid</code> field is
<code>-1</code>, automatic GID selection has to be done. The struct needs to be
updated to reflect this. (However, we can not update <code>struct vxpdb_user
.group</code> as it is a string, so the application needs to re-lookup the user
with the new UID.)</p>

<h1>Modifying a user or group</h1>

<div class="pleft2">
<table>
  <tr>
    <td class="code"><code><b>int</b> pdb_usermod(<b>void *</b>state, <b>struct</b> vxpdb_user <b>*</b>user, <b>struct</b> vxpdb_user <b>*</b>mask);<br />
<b>int</b> pdb_groupmod(<b>void *</b>state, <b>struct</b> vxpdb_group <b>*</b>user, <b>struct</b> vxpdb_group <b>*</b>mask);</code></td>
  </tr>
</table>
</div>

<p class="block"><code>b_usermod()</code> searches for the next matching
user/group and modifies its account information. <code>NULL</code> or
<code>-1</code> fields (respectively) mean ignore, for both search mask
(<code>user</code>) and modification mask (<code>mask</code>).</p>

<p class="block">You should take care that the search mask does not match
multiple users, otherwise it is undefined which user that could match is
modified.</p>

<h1>Deleting a user or group</h1>

<div class="pleft2">
<table>
  <tr>
    <td class="code"><code><b>int</b> pdb_userdel(<b>void *</b>state, <b>struct</b> vxpdb_user <b>*</b>user);<br />
<b>int</b> pdb_groupdel(<b>void *</b>state, <b>struct</b> vxpdb_group <b>*</b>group);</code></td>
  </tr>
</table>
</div>

<p class="block">Deletes the [first] user/group matching
<code>user</code>/<code>group</code>, respectively.</p>

<h1>Module info</h1>

<p class="block">Beautify the module by using the macros
<code>MODULE_NAME(string)</code>, <code>MODLUE_DESC(string)</code> and/or
<code>MODULE_INFO(string)</code>. This is not mandatory, and applications must
handle this situation if <code>((<b>struct</b> vxpdb_state
<b>*</b>)m)<b>-&gt;</b>desc</code> is <code>NULL</code>.</p>

<h1>Module control interface</h1>

<p class="code"><code><b>int</b> pdb_modctl(<b>struct</b> vxpdb_state <b>*</b>mp, <b>long</b> request, <b>...</b>)</code></p>

<p class="block">The back-end can be controlled via the
<code>pdb_modctl()</code> call. (The idea is analogous to a device driver's
<code>ioctl()</code>.) There are some requests defined in
<code>libvxpdb.h</code>.</p>

<div class="pleft">
<table>
  <tr>
    <td class="code"><code>
pdb_modctl(<b>struct</b> vxpdb_state <b>*</b>mp, PDB_FLUSH);<br />
pdb_modctl(<b>struct</b> vxpdb_state <b>*</b>mp, PDB_NEXTUID_SYS);<br />
pdb_modctl(<b>struct</b> vxpdb_state <b>*</b>mp, PDB_NEXTUID);<br />
pdb_modctl(<b>struct</b> vxpdb_state <b>*</b>mp, PDB_NEXTGID_SYS);<br />
pdb_modctl(<b>struct</b> vxpdb_state <b>*</b>mp, PDB_NEXTGID);</code></td>
  </tr>
</table>
</div>

<div class="pleft">
<table border="1">
  <tr>
    <td class="t1"><code>ACCDB_FLUSHDB</code></td>
    <td class="t1">Causes any changes to be committed to the underlying
      layer. (That might not be hard disk!)</td>
  </tr>
  <tr>
    <td class="t2"><code>ACCDB_NEXTUID_SYS</code></td>
    <td class="t2">Returns the next free auto-UID below
      <code>UID_MIN</code></td>
  </tr>
  <tr>
    <td class="t1"><code>ACCDB_NEXTUID</code></td>
    <td class="t1">Returns the next free auto-UID within <code>UID_MIN</code>
      and <code>UID_MAX</code></td>
  </tr>
  <tr>
    <td class="t2"><code>ACCDB_NEXTGID_SYS</code></td>
    <td class="t2">Returns the next free auto-GID below
      <code>GID_MIN</code></td>
  </tr>
  <tr>
    <td class="t1"><code>ACCDB_NEXTGID</code></td>
    <td class="t1">Returns the next free auto-GID within <code>GID_MIN</code>
      and <code>GID_MAX</code></td>
  </tr>
</table>
</div>
-->
<?php include_once("Base-footer.php"); ?>
