<?php include_once("zheader.php"); ?>

<h1>Name <img src="d_arr.png" /></h1>

<p class="pleft">backend_api - The back-end module API, how it works, how to
write a module</p>

<h1>Description <img src="d_arr.png" /></h1>

<p class="block">Since there is a great variety of user databases, the Unified
Account Database provides a generic API to any application. Some user databases
are for example the Shadow Password System (<tt>/etc/passwd</tt> and friends).
Another could be the Samba userdb in <tt>/var/lib/samba</tt>, or OpenLDAP
(libldap).</p>

<p class="block">Depending on how the user database is organized, you might
want, or even need to read it into memory. Be sure to keep a lock on the
database and/or files when writing, if necessary to not run into race
conditions.</p>

<p class="block">One should choose wise how to access local files. (If that
applies to the module, e.g. <tt>accdb_shadow</tt>.) libc's <tt>putpwent()</tt>
is not widely available, and suffers from ancient code base and missing
duplicate checks. Doing the I/O on our own (and even caching if the module
author wants to) can result in a faster execution, and allows for way more
control. "Pipe modules", which just transfer data, as it would be with a LDAP
or MySQL database, do not need to do any direct file I/O, since the underlying
DB will handle that.</p>

<p class="block">Applications that wish to use userdb modules might want to use
<tt>accdb_load()</tt> from <a href="accdb_api.php">libaccdb</a> for
simplicity. However, they are not forced to do so, but then they would have to
lookup the module symbols themselves.</p>

<h1>Additional notes <img src="d_arr.png" /></h1>

<p class="block">Keep in mind that the back-end and the user database have to
support non-unique UIDs (i.e. multiple accounts with UID 0 root privilegues),
so you can not use UIDs as the sort key in any data structure which requires
unique keys, such as B-trees. (Exception: You could pun the B-tree structure to
do one level of redirection, i.e. object <tt>0</tt> is a pointer to an array of
one or more users.)</p>

<h1>Synopsis <img src="d_arr.png" /></h1>

<p class="block">First, include &lt;<tt>accdb.h</tt>&gt; to get at the
<tt>accdb_[ug]entry struct</tt>s. The module should provide following functions
to the outside world:</p>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>#</b>include &lt;accdb.h&gt;<br />
<br />
<b>int</b> am_init(<b>struct</b> ac_module <b>*</b>mp, <b>long</b> flags, <b>void *</b>data);<br />
<b>int</b> am_open(<b>struct</b> ac_module <b>*</b>mp, <b>long</b> flags, <b>void *</b>data);<br />
<b>int</b> am_close(<b>struct</b> ac_module <b>*</b>mp);<br />
<b>long</b> am_modctl(<b>struct</b> ac_module <b>*</b>mp, <b>long</b> req, <b>...</b>);<br />
<b>void</b> am_deinit(<b>struct</b> ac_module <b>*</b>mp);<br />
<br />
<b>int</b> am_useradd(<b>struct</b> ac_module <b>*</b>mp, <b>struct</b> ac_user <b>*</b>user);<br />
<b>int</b> am_usermod(<b>struct</b> ac_module <b>*</b>mp, <b>struct</b> ac_user <b>*</b>user, <b>struct</b> ac_user <b>*</b>mask);<br />
<b>int</b> am_userdel(<b>struct</b> ac_module <b>*</b>mp, <b>struct</b> ac_user <b>*</b>user);<br />
<b>int</b> am_usertrav(<b>struct</b> ac_module <b>*</b>mp, <b>struct</b> ac_user <b>*</b>result);<br />
<b>int</b> am_userinfo(<b>struct</b> ac_module <b>*</b>mp, <b>struct</b> ac_user <b>*</b>req, <b>struct</b> ac_user <b>*</b>dest, <b>size_t</b> s);<br />
<br />
<b>int</b> am_groupadd(<b>struct</b> ac_module <b>*</b>mp, <b>struct</b> ac_group <b>*</b>group);<br />
<b>int</b> am_groupmod(<b>struct</b> ac_module <b>*</b>mp, <b>struct</b> ac_group <b>*</b>group, <b>struct</b> ac_group <b>*</b>mask);<br />
<b>int</b> am_groupdel(<b>struct</b> ac_module <b>*</b>mp, <b>struct</b> ac_group <b>*</b>group);<br />
<b>int</b> am_grouptrav(<b>struct</b> ac_module <b>*</b>mp, <b>struct</b> ac_group <b>*</b>result);<br />
<b>int</b> am_groupinfo(<b>struct</b> ac_module <b>*</b>mp, <b>struct</b> ac_group <b>*</b>req, <b>struct</b> ac_group <b>*</b>dest, <b>size_t</b> s);<br />
</tt></td>
  </tr>
</table>
</div>

<p class="block"><tt>b_init()</tt> and <tt>b_deinit()</tt> do not need to be
provided by the back-end module. <tt>b_open()</tt> is not required either --
ACCDB will refuse to load the module, though. (See <a
href="accdb_api.php">doc/accdb_api.php</a> for more details.) All other
fuctions must be present, otherwise you will get a Segmentation Fault when
running an application which expected that function to be there.</p>

<p class="block">If you include &lt;<tt>accdb_int.h</tt>&gt;, you also get the
prototypes for free, besides some macros used for naming the module (see below
near "Module Info").</p>

<p class="block">The <tt>data</tt> parameter can contain backend-specific
options, though this should really only be used when loading a module by
explicit filename. The <tt>flags</tt> parameter can contain various
ACCDB-relevant flags, but there are currently none defined.</p>

<p class="pleft"><sup><b>1</b></sup> They are prefixed so they do not clash
with system function names.</p>

<h1>Initialization <img src="d_arr.png" /></h1>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>int</b> am_init(<b>struct</b> ac_module <b>*</b>mp, <b>long</b> flags, <b>void *</b>options);<br />
<b>int</b> am_open(<b>struct</b> ac_module <b>*</b>mp, <b>long</b> flags, <b>void *</b>options);<br />
<b>int</b> am_close(<b>struct</b> ac_module <b>*</b>mp);<br />
<b>void</b> am_deinit(<b>struct</b> ac_module <b>*</b>mp);</tt></td>
  </tr>
</table>
</div>

<p class="block"><tt>b_open()</tt> is passed a pointer to the
<tt>ac_module</tt> struct. You can allocate <tt>mp<b>-&gt;</b>state</tt> to
point to a user-defined struct. That is, a pointer to a private data area you
might need. That struct may hold anything you need. It could, for example,
contain <tt><b>FILE *</b></tt> pointers, or <tt><b>int</b></tt>s to file
descriptors (fds, sockets) connected to the user database. Using static
variables is all-round bad (for re-entrancy too), do not stick to it.</p>

<p class="block">The module may or may not allow opening the user database
twice (the module may be opened by different applications). You should handle
that case then. One, locking files, and two, checking for the lock is the usual
thing for a module to be opened only once at a time.</p>

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

<h1>Traversing the user and group lists <img src="d_arr.png" /></h1>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>int</b> am_usertrav(<b>void *</b>state, <b>struct</b> ac_user <b>*</b>result);<br />
<b>int</b> am_grouptrav(<b>void *</b>state, <b>struct</b> ac_group <b>*</b>result);</tt></td>
  </tr>
</table>
</div>

<p class="block">The <tt>accdb_[ug]entry <b>struct</b></tt>s are as
follows:</p>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>struct</b> ac_user {<br />
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
<b>struct</b> ac_group {<br />
&nbsp; &nbsp; <b>long</b> gid;<br />
&nbsp; &nbsp; <b>char *</b>gname;<br />
};</tt></td>
  </tr>
</table>
</div>

<p class="block">Analogous to <tt>getpwent()</tt>, just a bit more organized,
is the <tt>b_usertrav()</tt> function. It takes the usual state pointer and a
<tt><b>struct</b> ac_user <b>*</b></tt> pointer. When calling
<tt>b_usertrav()</tt>, it takes the next user found in the database and fills
in the struct.</p>

<p class="block">All the <tt><b>char *</b></tt> fields in the structs shall
point to allocated memory (or memory available throughout the program), so do
not use local variables or <tt>alloca()</tt>. <tt>strdup()</tt> or similar
might help you. Those strings may not be changed by anything else than the
backends module, they are meant to be read-only. That way, these strings can be
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

<h1>Retrieve info about a user or group <img src="d_arr.png" /></h1>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>int</b> am_userinfo(<b>void *</b>state, <b>struct</b> ac_user <b>*</b>req, <b>struct</b> ac_user <b>*</b>dest, <b>size_t</b> s);<br />
<b>int</b> am_groupinfo(<b>void *</b>state, <b>struct</b> ac_group <b>*</b>req, <b>struct</b> ac_group <b>*</b>dest, <b>size_t</b> s);<br /></tt></td>
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

<h1>Adding a user or group <img src="d_arr.png" /></h1>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>int</b> am_useradd(<b>void *</b>state, <b>struct</b> ac_user <b>*</b>user);<br />
<b>int</b> am_groupadd(<b>void *</b>state, <b>struct</b> ac_group <b>*</b>group);<br /></tt></td>
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
reflect this. (However, we can not update <tt>struct ac_user .group</tt> as
it is a string, so the application needs to re-lookup the user with the new
UID.)</p>

<h1>Modifying a user or group <img src="d_arr.png" /></h1>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>int</b> am_usermod(<b>void *</b>state, <b>struct</b> ac_user <b>*</b>user, <b>struct</b> ac_user <b>*</b>mask);<br />
<b>int</b> am_groupmod(<b>void *</b>state, <b>struct</b> ac_group <b>*</b>user, <b>struct</b> ac_group <b>*</b>mask);</tt></td>
  </tr>
</table>
</div>

<p class="block"><tt>b_usermod()</tt> searches for the next matching user/group
and modifies its account information. <tt><b>NULL</b></tt> or <tt>-1</tt>
fields (respectively) mean ignore, for both search mask (<tt>user</tt>) and
modification mask (<tt>mask</tt>).</p>

<p class="block">You should take care that the search mask does not match
multiple users, otherwise it is undefined which user that could match is
modified.</p>

<h1>Deleting a user or group <img src="d_arr.png" /></h1>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>int</b> am_userdel(<b>void *</b>state, <b>struct</b> ac_user <b>*</b>user);<br />
<b>int</b> am_groupdel(<b>void *</b>state, <b>struct</b> ac_group <b>*</b>group);</tt></td>
  </tr>
</table>
</div>

<p class="block">Deletes the [first] user/group matching
<tt>user</tt>/<tt>group</tt>, respectively.</p>

<h1>Module info <img src="d_arr.png" /></h1>

<p class="block">Beautify the module by using the macros
<tt>MODULE_NAME(string)</tt>, <tt>MODLUE_DESC(string)</tt> and/or
<tt>MODULE_INFO(string)</tt>. This is not mandatory, and applications must
handle this situation if <tt>((<b>struct</b> ac_module
<b>*</b>)m)<b>-&gt;</b>desc</tt> is <tt>NULL</tt>.</p>

<h1>Module control interface <img src="d_arr.png" /></h1>

<div class="pleft2">
<table>
  <tr>
    <td class="cmd"><tt><b>int</b> am_modctl(<b>struct</b> ac_module <b>*</b>mp,
      <b>long</b> req, <b>...</b>)</tt></td>
  </tr>
</table>
</div>

<p class="block">The back-end can be controlled via the <tt>b_modctl()</tt>
call. There are some requests defined in <tt>accdb.h</tt>; General Modctl()s
get a number between <tt>0x1</tt> and <tt>0xFF</tt>, Extension Modctl()s
between <tt>0x100</tt> and <tt>0xBFFF</tt> and back-end specific ones from
<tt>0xC000</tt> (mostly debugging purposes only).</p>

<div class="pleft">
<table>
  <tr>
    <td class="cmd"><tt>b_modctl(<b>struct</b> ac_module <b>*</b>mp, ACCDB_ADDFLAGS, <b>unsigned long</b> flagmask);<br />
b_modctl(<b>struct</b> ac_module <b>*</b>mp, ACCDB_DELFLAGS, <b>unsigned long</b> flagmask);<br />
b_modctl(<b>struct</b> ac_module <b>*</b>mp, ACCDB_FLUSHDB);<br />
b_modctl(<b>struct</b> ac_module <b>*</b>mp, ACCDB_NEXTUID_SYS);<br />
b_modctl(<b>struct</b> ac_module <b>*</b>mp, ACCDB_NEXTUID);<br />
b_modctl(<b>struct</b> ac_module <b>*</b>mp, ACCDB_NEXTGID_SYS);<br />
b_modctl(<b>struct</b> ac_module <b>*</b>mp, ACCDB_NEXTGID);</tt></td>
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

<?php include_once("zfooter.php"); ?>
