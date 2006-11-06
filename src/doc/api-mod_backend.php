<?php include_once("Base-header.php"); ?>

<h1>Module definition</h1>

<p class="block">Each module must define a structure <tt>struct
vxpdb_mvtable</tt> called <tt>THIS_MODULE</tt> in which they set the function
pointers to the respective functions. The module structure also includes space
for name, author and description of the module for display with the
<i>pdbinfo</i> utility. A reduced example definition could look like this:</p>

<p class="code"><tt><b>#</b>include "drivers/static-build.h"<br />
<b>#</b>include &lt;vitalnix/libvxpdb/libvxpdb.h&gt;<br />
<br />
COND_STATIC <b>struct</b> vxpdb_mvtable THIS_MODULE <b>=</b> {<br />
&nbsp; &nbsp; .name &nbsp; &nbsp; &nbsp;<b>=</b> "Our sample module",<br />
&nbsp; &nbsp; .userinfo &nbsp;<b>=</b> our_userinfo,<br />
&nbsp; &nbsp; .groupinfo <b>=</b> our_groupinfo,<br />
};<br />
<br />
STATIC_REGISTER(our, <b>&amp;</b>THIS_MODULE);</tt></p>

<p class="block">The <tt>drivers/static-build.h</tt> include, the
<tt>COND_STATIC</tt> and <tt>STATIC_REGISTER</tt> macros expand to extra code
and/or initializers that are needed if the module should be built into
<i>libvxpdb</i> (or not). (The struct must be non-static if the module shall be
a shared library, and must be static if the module is compiled into
<i>libvxpdb</i>&nbsp;-- to make <tt>THIS_MODULE</tt> available and to avoid
multiple global <tt>THIS_MODULE</tt> symbols, respectively.)</p>

<p class="block">Then of course, the module needs to provide the functions we
have just named in <tt>THIS_MODULE</tt>. They can then be called from user
applications using the <tt>vxpdb_*()</tt> functions and the respective instance
as obtained from <tt>vxpdb_load()</tt>. Note that the <tt>THIS_MODULE</tt>
struct <i>must</i> be writable, because <tt>vxpdb_load()</tt> will exchange all
<tt>NULL</tt> pointers to a respective dummy function (as defined in
<tt>libvxpdb/loader.c</tt> in the source tree).</p>

<!--
<p class="block">In the following sections, the prototype for the module
function is shown, and it is explained what the function should do. Actually,
our module does not need to provide ANY functions whatsoever, in which case the
resulting module does nothing, like <i>libvxdb_dummy</i>.</p>

<p class="block">All functions that return an integer follow the following
return code strategy: Return values less than or equal zero are considered
error values that are the negative of <tt>errno</tt>. (Yes, that leaves room
for the exact interpretation of zero, but it is an error.) For success, a
positive non-zero value should be returned, usually <tt>1</tt> for
simplicity.</p> Functions that do return a count of something, e.g. number of
users, can legitimately return any positive number or zero. These are modeled
after the return values of the <tt>open()</tt> library call.</p>

<h2>-&gt;init</h2>

<p class="code"><tt><b>int (*</b>init<b>)</b>(<b>struct</b> vxpdb_state <b>*</b>mip, <b>void *</b>private_data);</tt></p>

<p class="block"><tt>our_init()</tt> function gets called after the shared
library has been opened through <tt>pdb_load()</tt> from the caller program. It
gets passed the <tt>priv</tt> argument from <tt>pdb_load()</tt>, which may
contain module-specific initialization data. Most of the times however, the
caller will just put <tt>NULL</tt> for <tt>private_data</tt>.</p>

<p class="block">In this function, the module should allocate its state and
read configuration files. The allocation of a state might look like below, and
is necessary for reentrancy:</p>

<p class="code"><tt><b>static int</b> our_init(<b>struct</b> vxpdb_state <b>*</b>mip, <b>void *</b>priv) {<br />
    <b>struct</b> our_state <b>*</b>state;<br />
    state <b>=</b> mip-&gt;state <b>=</b> malloc(sizeof(struct our_state));<br />
    state-&gt;config <b>=</b> read_some_config("bla.conf");<br />
    read_some_extras(priv);<br />
    return 1;<br />
}</tt></p>

<p class="block">This is a very basic example, and you should look at the
already existing modules to see what they do, and possibly how they do it.</p>

<h2>-&gt;open</h2>

<p class="code"><tt><b>int (*</b>open<b>)</b>(<b>struct</b> vxpdb_state <b>*</b>mip, <b>long</b> flags);</tt></p>

<p class="block">The open function should open a connection to the password
database (if applicable), or do whatever is equivalent to prepare further
actions. The <tt>flags</tt> parameter is explained in
<tt>api-libvxpdb</tt>. Some sample code:</p>

<p class="code"><tt><b>int</b> our_open(<b>struct</b> vxpdb_state <b>*</b>mip, <b>long</b> flags) {<br />
    <b>struct</b> our_state <b>*</b>state <b>=</b> mip-&gt;state;<br />
    if((state-&gt;fp <b>=</b> fopen("/etc/passwd", "r")) <b>==</b> NULL)<br />
        return -errno;<br />
    return 1;<br />
}</tt></p>

<h2>-&gt;close</h2>

<p class="code"><tt><b>void (*</b>close<b>)</b>(<b>struct</b> vxpdb_state <b>*</b>mip);</tt></p>

<h2>-&gt;deinit</h2>

<p class="code"><tt><b>void (*</b>deinit<b>)</b>(<b>struct</b> vxpdb_state <b>*</b>mip);</tt></p>

<h2>-&gt;modctl</h2>

<p class="code"><tt><b>long (*</b>modctl<b>)</b>(<b>struct</b> vxpdb_state <b>*</b>mip, <b>long</b> command, <b>...</b>);</tt></p>

<h2>-&gt;lock</h2>

<p class="code"><tt><b>int (*</b>lock<b>)</b>(<b>struct</b> vxpdb_state <b>*</b>mip);</tt></p>

<p class="block">

<h2>-&gt;unlock</h2>

<p class="code"><tt><b>int (*</b>unlock<b>)</b>(<b>struct</b> vxpdb_state <b>*</b>mip);</tt></p>

<h1></h1>

<h1></h1>

<h1></h1>

<h1></h1>

<h1>Description</h1>

<p class="block">libvxpdb Since there is a great variety of user databases, the
Unified Account Database provides a generic API to any application. Some user
databases are for example the Shadow Password System (<tt>/etc/passwd</tt> and
friends). Another could be the Samba userdb in <tt>/var/lib/samba</tt>, or
OpenLDAP (libldap).</p>

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

<!--
<p class="block">Upon success, return <tt>&gt;0</tt>, otherwise return
<tt>0</tt> (and possibly set <tt>errno</tt>), or even return <tt>-errno</tt>
and set <tt>errno</tt> to signalize a hard error (i.e. <tt>ENOMEM</tt> due to
<tt>malloc()</tt>).</p>

<p class="block"><tt>b_close()</tt> shall deinitialize the currently open
session for the user database. Flush any data to disk if needed and free up all
memory used for the session.</p>

<p class="block"><tt>b_deinit()</tt> is the counterpart of <tt>b_init()</tt>
and is called through <tt>accdb_unload()</tt>. This function also does not
necessarily need to exist either.</p>

<h1>Traversing the user and group lists</h1>

<p class="code"><tt><b>int</b> pdb_usertrav(<b>void *</b>state, <b>struct</b> vxpdb_user <b>*</b>result);<br />
<b>int</b> pdb_grouptrav(<b>void *</b>state, <b>struct</b> vxpdb_group <b>*</b>result);</tt></p>

<p class="block">The <tt>accdb_[ug]entry <b>struct</b></tt>s are as
follows:</p>

<p class="code"><tt><b>struct</b> vxpdb_user {<br />
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
};</tt></p>

<p class="block">Analogous to <tt>getpwent()</tt>, just a bit more organized,
is the <tt>b_usertrav()</tt> function. It takes the usual state pointer and a
<tt><b>struct</b> vxpdb_user <b>*</b></tt> pointer. When calling
<tt>b_usertrav()</tt>, it takes the next user found in the database and fills
in the struct.</p>

<p class="block">All the <tt><b>char *</b></tt> fields in the structs shall
point to allocated memory (or memory available throughout the program), so do
not use local variables or <tt>alloca()</tt>. <tt>strdup()</tt> or similar
might help you. Those strings may not be changed by anything else than the
driver module, they are meant to be read-only. That way, these strings can be
reused anywhere.</p>

<p class="block">For each successful returned user via <tt>b_usertrav()</tt>,
the function shall return <tt>&gt;0</tt> (usually <tt>1</tt> suffices). The
order of users is of no importance; if necessary, the application itself will
need to sort it. If the end of the list is reached, return <tt>0</tt>, or, if
an error occurred, set <tt>errno</tt> and return <tt>-errno</tt>. When the end
of the list is reached, <tt>b_usertrav()</tt> can be called with the
<tt>result</tt> parameter set to <tt>NULL</tt> to reset the list traverser.</p>

<p class="block">Similar applies to the group traversing function
<tt>b_grouptrav()</tt>.</p>

<p class="block">The traversion pointer is not static but stored in the
state.</p>

<h1>Retrieve info about a user or group</h1>

<div class="pleft2">
<table>
  <tr>
    <td class="code"><tt><b>int</b> pdb_userinfo(<b>void *</b>state, <b>struct</b> vxpdb_user <b>*</b>req, <b>struct</b> vxpdb_user <b>*</b>dest, <b>size_t</b> s);<br />
<b>int</b> pdb_groupinfo(<b>void *</b>state, <b>struct</b> vxpdb_group <b>*</b>req, <b>struct</b> vxpdb_group <b>*</b>dest, <b>size_t</b> s);<br /></tt></td>
  </tr>
</table>
</div>

<p class="block"><tt>b_userinfo()</tt> walks down the list of users searching
for matching users. The contents of <tt>req</tt> are compared with every user
in the list. If a value in <tt>req</tt> is <tt>-1</tt> or <tt>NULL</tt>
(depending on the member type and meaning), it is ignored and not included in
the comparison, so you could do i.e. the following searches:</p>

<p class="pleft2">any user with UID 37007 (<tt>req<b>-&gt;</b>lname <b>== NULL
&amp;&amp;</b> req<b>-&gt;</b>uid <b>==</b> 37007</tt>),<br />

user "jengelh", any UID (<tt>strcmp(req<b>-&gt;</b>lname, "jengelh") == 0
<b>&amp;&amp;</b> req<b>-&gt;</b>uid <b>==</b> -1</tt>)<br />

user "jengelh" with UID 37007 (<tt>strcmp(req<b>-&gt;</b>lname, "jengelh")
<b>==</b> 0 <b>&amp;&amp;</b> req<b>-&gt;</b>uid <b>==</b> 37007</tt>)</p>

<p class="block"><tt>b_userinfo()</tt> has three different operation cases. The
first is if <tt>dest</tt> is not <tt>NULL</tt>, in which case at most
<tt>s</tt> users are placed into <tt>dest</tt>, and the number of users stored
(can never be <tt>&gt;s</tt>, but may be less) is returned. Case two is that
<tt>dest</tt> is <tt>NULL</tt> and <tt>s</tt> is 0, where <tt>1</tt> is
returned as soon as the first match is found. Case three, <tt>dest</tt> is
<tt>NULL</tt> and <tt>s</tt> is 1, no users are copied, but the number of
matches will be returned.</p>

<p class="block">Again, the complement <tt>b_groupinfo()</tt> behaves just as
like.</p>

<p class="block"><tt>b_userinfo()</tt> and <tt>b_groupinfo()</tt> do not
interrupt the traversion with <tt>b_usertrav()</tt> / <tt>b_grouptrav()</tt>.
They use their own (local) traversion pointer.</p>

<h1>Adding a user or group</h1>

<div class="pleft2">
<table>
  <tr>
    <td class="code"><tt><b>int</b> pdb_useradd(<b>void *</b>state, <b>struct</b> vxpdb_user <b>*</b>user);<br />
<b>int</b> pdb_groupadd(<b>void *</b>state, <b>struct</b> vxpdb_group <b>*</b>group);<br /></tt></td>
  </tr>
</table>
</div>

<p class="block"><tt>b_useradd</tt> creates a new user in the database from the
data provided in <tt>user</tt>. It duplicates all strings provided by an
application and adds them to the database.</p>

<p class="block">If <tt>.lname</tt> is <tt>NULL</tt>, or there is no group
specification in either <tt>.gid</tt> or <tt>.igrp</tt>, <tt>errno</tt> is to
be set to <tt>EINVAL</tt> and <tt>0</tt> is to be returned. For
<tt>b_groupadd()</tt>, <tt>errno</tt> is set to <tt>EINVAL</tt> and <tt>0</tt>
is returned if <tt>.gname</tt> is <tt>NULL</tt>. <tt>-errno</tt> is returned
when something underlying <tt>b_useradd()</tt> (or <tt>b_groupadd()</tt>)
failed, like writing to the database.</p>

<p class="block">If the <tt>.uid</tt> or <tt>.gid</tt> field is <tt>-1</tt>,
automatic GID selection has to be done. The struct needs to be updated to
reflect this. (However, we can not update <tt>struct vxpdb_user .group</tt> as
it is a string, so the application needs to re-lookup the user with the new
UID.)</p>

<h1>Modifying a user or group</h1>

<div class="pleft2">
<table>
  <tr>
    <td class="code"><tt><b>int</b> pdb_usermod(<b>void *</b>state, <b>struct</b> vxpdb_user <b>*</b>user, <b>struct</b> vxpdb_user <b>*</b>mask);<br />
<b>int</b> pdb_groupmod(<b>void *</b>state, <b>struct</b> vxpdb_group <b>*</b>user, <b>struct</b> vxpdb_group <b>*</b>mask);</tt></td>
  </tr>
</table>
</div>

<p class="block"><tt>b_usermod()</tt> searches for the next matching user/group
and modifies its account information. <tt>NULL</tt> or <tt>-1</tt> fields
(respectively) mean ignore, for both search mask (<tt>user</tt>) and
modification mask (<tt>mask</tt>).</p>

<p class="block">You should take care that the search mask does not match
multiple users, otherwise it is undefined which user that could match is
modified.</p>

<h1>Deleting a user or group</h1>

<div class="pleft2">
<table>
  <tr>
    <td class="code"><tt><b>int</b> pdb_userdel(<b>void *</b>state, <b>struct</b> vxpdb_user <b>*</b>user);<br />
<b>int</b> pdb_groupdel(<b>void *</b>state, <b>struct</b> vxpdb_group <b>*</b>group);</tt></td>
  </tr>
</table>
</div>

<p class="block">Deletes the [first] user/group matching
<tt>user</tt>/<tt>group</tt>, respectively.</p>

<h1>Module info</h1>

<p class="block">Beautify the module by using the macros
<tt>MODULE_NAME(string)</tt>, <tt>MODLUE_DESC(string)</tt> and/or
<tt>MODULE_INFO(string)</tt>. This is not mandatory, and applications must
handle this situation if <tt>((<b>struct</b> vxpdb_state
<b>*</b>)m)<b>-&gt;</b>desc</tt> is <tt>NULL</tt>.</p>

<h1>Module control interface</h1>

<p class="code"><tt><b>int</b> pdb_modctl(<b>struct</b> vxpdb_state <b>*</b>mp, <b>long</b> request, <b>...</b>)</tt></p>

<p class="block">The back-end can be controlled via the <tt>pdb_modctl()</tt>
call. (The idea is analogous to a device driver's <tt>ioctl()</tt>.) There are
some requests defined in <tt>libvxpdb.h</tt>.</p>

<div class="pleft">
<table>
  <tr>
    <td class="code"><tt>
pdb_modctl(<b>struct</b> vxpdb_state <b>*</b>mp, PDB_ADDFLAGS, <b>unsigned long</b> flagmask);<br />
pdb_modctl(<b>struct</b> vxpdb_state <b>*</b>mp, PDB_DELFLAGS, <b>unsigned long</b> flagmask);<br />
pdb_modctl(<b>struct</b> vxpdb_state <b>*</b>mp, PDB_FLUSH);<br />
pdb_modctl(<b>struct</b> vxpdb_state <b>*</b>mp, PDB_NEXTUID_SYS);<br />
pdb_modctl(<b>struct</b> vxpdb_state <b>*</b>mp, PDB_NEXTUID);<br />
pdb_modctl(<b>struct</b> vxpdb_state <b>*</b>mp, PDB_NEXTGID_SYS);<br />
pdb_modctl(<b>struct</b> vxpdb_state <b>*</b>mp, PDB_NEXTGID);</tt></td>
  </tr>
</table>
</div>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1"><tt>ACCDB_ADDFLAGS</tt></td>
    <td class="t1">The provided flags are added (binary OR) to the state's
      flags.</td>
  </tr>
  <tr>
    <td class="t2"><tt>ACCDB_DELFLAGS</tt></td>
    <td class="t2">The provided flags are removed from the state's flags.</td>
  </tr>
  <tr>
    <td class="t1"><tt>ACCDB_FLUSHDB</tt></td>
    <td class="t1">Causes any changes to be committed to the underlying
      layer. (That might not be hard disk!)</td>
  </tr>
  <tr>
    <td class="t2"><tt>ACCDB_NEXTUID_SYS</tt></td>
    <td class="t2">Returns the next free auto-UID below <tt>UID_MIN</tt></td>
  </tr>
  <tr>
    <td class="t1"><tt>ACCDB_NEXTUID</tt></td>
    <td class="t1">Returns the next free auto-UID within <tt>UID_MIN</tt> and
      <tt>UID_MAX</tt></td>
  </tr>
  <tr>
    <td class="t2"><tt>ACCDB_NEXTGID_SYS</tt></td>
    <td class="t2">Returns the next free auto-GID below <tt>GID_MIN</tt></td>
  </tr>
  <tr>
    <td class="t1"><tt>ACCDB_NEXTGID</tt></td>
    <td class="t1">Returns the next free auto-GID within <tt>GID_MIN</tt> and
      <tt>GID_MAX</tt></td>
  </tr>
</table>
</div>
-->
<?php include_once("Base-footer.php"); ?>
