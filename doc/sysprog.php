<?php include_once("zheader.php"); ?>

<h1>Overview <img src="d_arr.png" /></h1>

<p class="block">This chapter contains the "manpages" for Vitalnix's system
programs.</p>

<p class="block">The utilities provided with Vitalnix try to behave like the
ones from the Shadow Password Suite package, to ensure compatibility. They also
have some new options, too.</p>

<p class="block"><b>NOTE:</b> Configuration files are read from
<tt>sysprog/../vetc/</tt>, too (if they exist there). Why not simply
<tt>vetc/</tt> you may ask? Because the former gives us a bit more security
when you have a random file which has exactly our name.</p>

<h1>Placeholders <img src="d_arr.png" /></h1>

<p class="block">For the <tt>-A</tt> and <tt>-B</tt> commands that can be run,
placeholders similar to <tt>printf(3)</tt>, i.e. put <tt>%l</tt> somewhere in
the string, and it will get replaced with the login name when it is executed.
You will need <b><tt>%%</tt></b> to get a literal percent sign, <tt>%</tt>.</p>

<p class="block">The following specifiers are available in the given
programs:</p>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1" rowspan="2">Spec</td>
    <td class="t1" rowspan="2">Description</td>
    <td class="t1" colspan="3">vuser...</td>
    <td class="t1" colspan="3">vgroup...</td>
  </tr>
  <tr>
    <td class="t1">add</td>
    <td class="t1">mod</td>
    <td class="t1">del</td>
    <td class="t1">add</td>
    <td class="t1">mod</td>
    <td class="t1">del</td>
  </tr>
  <tr>
    <td class="t2"><tt>%l</tt></td>
    <td class="t2">Login name</td>
    <td class="t2"><input type="checkbox" checked /></td>
    <td class="t2"><input type="checkbox" checked /></td>
    <td class="t2"><input type="checkbox" checked /></td>
    <td class="t2"><input type="checkbox" /></td>
    <td class="t2"><input type="checkbox" /></td>
    <td class="t2"><input type="checkbox" /></td>
  </tr>
  <tr>
    <td class="t1"><tt>%u</tt></td>
    <td class="t1">UID of the user</td>
    <td class="t1"><input type="checkbox" checked /></td>
    <td class="t1"><input type="checkbox" checked /></td>
    <td class="t1"><input type="checkbox" /></td>
    <td class="t1"><input type="checkbox" /></td>
    <td class="t1"><input type="checkbox" /></td>
    <td class="t1"><input type="checkbox" /></td>
  </tr>
  <tr>
    <td class="t2"><tt>%g</tt></td>
    <td class="t2">GID of the user/group</td>
    <td class="t2"><input type="checkbox" /></td>
    <td class="t2"><input type="checkbox" /></td>
    <td class="t2"><input type="checkbox" /></td>
    <td class="t2"><input type="checkbox" checked /></td>
    <td class="t2"><input type="checkbox" checked /></td>
    <td class="t2"><input type="checkbox" /></td>
  </tr>
  <tr>
    <td class="t1"><tt>%G</tt></td>
    <td class="t1">Group name (of users's primary group)</td>
    <td class="t1"><input type="checkbox" checked /></td>
    <td class="t1"><input type="checkbox" checked /></td>
    <td class="t1"><input type="checkbox" checked /></td>
    <td class="t1"><input type="checkbox" /></td>
    <td class="t1"><input type="checkbox" /></td>
    <td class="t1"><input type="checkbox" /></td>
  </tr>
</table>
</div>

<h1>useradd - Create a new user <img src="d_arr.png" /></h1>

<div class="pleft">
<table class="bordered">
  <tr>
    <td class="cmd"><tt><b>vuseradd</b> [-A [command]] [-B [command]] [-I] [-M
      backend] [-S] [-G group[,...]] [-c comment] [-d home_dir] [-e date] [-f
      inactive_days] [-g group] [-m [-k skel_dir]] [-p passwd] [-s shell] [-u
      uid [-o]|-r] login</tt></td>
  </tr>
</table>
</div>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1">-A [command]</td>
    <td class="t1">Runs the specified command after the user was successfully
      added. If <tt>-A</tt> is given, but with a zero-length command string,
      the default command in the configuration file is not run.</td>
  </tr>
  <tr>
    <td class="t2">-B [command]</td>
    <td class="t2">Runs the specified command before the user is added. It is
      run always, even if adding fails. If <tt>-B</tt> is given, but with a
      zero-length command string, the default command in the configuration file
      is not run.</td>
  </tr>
  <tr>
    <td class="t1">-I</td>
    <td class="t1">Interactively prompt for the new user's parameters</td>
  </tr>
  <tr>
    <td class="t2">-M backend</td>
    <td class="t2">Uses the specified backend rather than the default one
      listed in the ACCDB configuration file</td>
  </tr>
  <tr>
    <td class="t1">-S</td>
    <td class="t1">Uses the "split home" feature. This will create the home
      directory as <tt>/home/u/username</tt> rather than the default
      <tt>/home/username</tt>. Specifying <tt>-S</tt> twice (which is the
      maximum) will result in a two-level split, i.e.
      <tt>/home/u/us/username</tt>.  The <tt>-d</tt> option overrides this
      one.</td>
  </tr>
  <tr>
    <td class="t2">-G group[,...]</td>
    <td class="t2">The supplemental (secondary) groups the user is a member in,
      separated by comma. <tt>group</tt> can either be the group's name or its
      GID. <b><i>(UNSUPPORTED)</i></b></td>
  </tr>
  <tr>
    <td class="t1">-c comment</td>
    <td class="t1">The comment field for the new user. It can be in turn again
      split up in fields, separated by comma. Usually, the real name of the
      user is stored in the first field, the others are left for other
      commentary or data.</td>
  </tr>
  <tr>
    <td class="t2">-d dir</td>
    <td class="t2">The new user will get <tt>dir</tt> as his home directory.
      This option alone does not create his home directory, but rather only
      writes that directory information into the user database. You usually
      want to pass <tt>-m</tt> also along to create the directory. This option
      overrides <tt>-S</tt>.</td>
  </tr>
  <tr>
    <td class="t1">-e date</td>
    <td class="t1">Date on which the account expires. Date formats can be
      DD.MM.YYYY, MM/DD/YYYY or YYYY-MM-DD. The default is to not let the
      account expire.</td>
  </tr>
  <tr>
    <td class="t2">-f days</td>
    <td class="t2">The number of days which need to pass after a password
      expiration (that is not <tt>-e</tt>!) until the account is locked
      down.</td>
  </tr>
  <tr>
    <td class="t1">-g group</td>
    <td class="t1">The group name or number of the user's initial (primary)
      group. <tt>group</tt> can either be the group's name or its GID.</td>
  </tr>
  <tr>
    <td class="t2">-k dir</td>
    <td class="t2">The skeleton directory to use for the new user. Contents
      from this directory will be copied into the user's.</td>
  </tr>
  <tr>
    <td class="t1">-m</td>
    <td class="t1">Create the user's home directory. The default is to create a
      directory called the same name as the user in <tt>/home</tt>, or what you
      specified as <tt>HOME</tt> in the configuration file, or if you passed
      <tt>-S</tt> or <tt>-d</tt>. If this option is given, an implicit <tt>-k
      /etc/skel</tt> (or <tt>SKEL</tt>) takes effect, unless an explicit
      <tt>-k</tt> is specified.</td>
  </tr>
  <tr>
    <td class="t2">-o</td>
    <td class="t2">If a user with the explicit UID given by <tt>-u</tt> already
      exists, you can override the error message with this option, to create
      non-unique UIDs.</td>
  </tr>
  <tr>
    <td class="t1">-p pass</td>
    <td class="t1">The password to use for the new user. It must already be
      encrypted when passed with <tt>-p</tt>.</td>
  </tr>
  <tr>
    <td class="t2">-r</td>
    <td class="t2">Create a system user, that is, with a lower UID than the
      hardcoded default (100), or what is specified for <tt>MIN_UID</tt> in the
      autouid configuration file. The <tt>-u</tt> option overrides this
      one.</td>
  </tr>
  <tr>
    <td class="t1">-s shell</td>
    <td class="t1">Use <tt>shell</tt> as the default command interpreter for
      the new user. The default is <tt>/bin/bash</tt> or whatever you specified
      as <tt>SHELL</tt> in the configuration file.</td>
  </tr>
  <tr>
    <td class="t2">-u uid</td>
    <td class="t2">The numerical user id of the new user. The default is to
      assign the user the next greatest UID. This option overrides
      <tt>-r</tt>.</td>
  </tr>
</table>
</div>

<p class="block"><tt>useradd</tt> will create a new user using the supplied
parameters, and, if <tt>-I</tt> is given, interactively asks the user for the
parameters.
In interactive mode, each specified option is printed with a default value in
square brackets. If an empty string is entered, the default value is taken. The
<tt>-A</tt>, <tt>-B</tt>, <tt>-I</tt>, <tt>-M</tt> and <tt>-S</tt> options are
unique to Vitalnix. Shadow's <tt>-D</tt> option is not provided. The
configuration file for <tt>useradd</tt> is
<tt>sysprog/../vetc/useradd.conf</tt>, or <tt>/etc/useradd.conf</tt>.</p>

<h1>usermod - Modify a user account <img src="d_arr.png" /></h1>

<div class="pleft">
<table>
  <tr>
    <td class="cmd"><tt><b>vusermod</b> [-A [command]] [-B [command]] [-M
      backend] [-G group,[...]] [-L] [-U] [-c comment] [-d home_dir [-m]] [-e
      date] [-f inactive_days] [-g initial_group] [-G group,[...]] [-l
      login_name] [-p passwd] [-s shell] [-u uid [-o]] login</tt></td>
  </tr>
</table>
</div>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1">-A [command]</td>
    <td class="t1">Runs the specified command after the user was successfully
      added. If <tt>-A</tt> is given, but with a zero-length command string,
      the default command in the configuration file is not run.</td>
  </tr>
  <tr>
    <td class="t2">-B [command]</td>
    <td class="t2">Runs the specified command before the user is added. It is
      run always, even if adding fails. If <tt>-B</tt> is given, but with a
      zero-length command string, the default command in the configuration file
      is not run.</td>
  </tr>
  <tr>
    <td class="t1">-I</td>
    <td class="t1">Interactively prompt for the new parameters</td>
  </tr>
  <tr>
    <td class="t2">-M backend</td>
    <td class="t2">Uses the specified backend rather than the default one
      listed in the ACCDB configuration file</td>
  </tr>
  <tr>
    <td class="t1">-G group[,...]</td>
    <td class="t1">Changes the user's info about which supplemental (secondary)
      groups he belongs to. Note that this will cancel his membership in any of
      the groups not specified, so you to add a user to another group, you need
      to run <tt>usermod</tt> with <tt>-G first</tt> and the second time with
      <tt>-G first,second</tt>. <b><i>(UNSUPPORTED)</i></b></td>
  </tr>
  <tr>
    <td class="t2">-L</td>
    <td class="t2">Lock the user's account (prohibit logins)
      <b><i>(UNSUPPORTED)</i></b></td>
  </tr>
  <tr>
    <td class="t1">-U</td>
    <td class="t1">Unlock the user's account <b><i>(UNSUPPORTED)</i></b></td>
  </tr>
  <tr>
    <td class="t2">-c comment</td>
    <td class="t2">Changes the comment (GECOS) field</td>
  </tr>
  <tr>
    <td class="t1">-d home</td>
    <td class="t1">Changes the home directory as found in the user database.
      Also see <tt>-m</tt>.</td>
  </tr>
  <tr>
    <td class="t2">-e date</td>
    <td class="t2">The new date on which the user account will expire. Valid
      formats are DD.MM.YYYY, MM/DD/YYYY and YYYY-MM-DD.</td>
  </tr>
  <tr>
    <td class="t1">-f days</td>
    <td class="t1">The number of days which need to pass after a password
      expiration (that is not <tt>-e</tt>!) until the account is locked
      down.</td>
  </tr>
  <tr>
    <td class="t2">-g group</td>
    <td class="t2">Changes the users initial (primary) group</td>
  </tr>
  <tr>
    <td class="t1">-l login</td>
    <td class="t1">Changes the login name for the user, which must be
      unique.</td>
  </tr>
  <tr>
    <td class="t2">-m</td>
    <td class="t2">When <tt>-d</tt> is given, specifying <tt>-m</tt> will
      move data from the old home directory into the new one, necessarily
      creating the new directory if it does not exist yet.
      <b><i>(UNSUPPORTED)</i></b></td>
  </tr>
  <tr>
    <td class="t1">-o</td>
    <td class="t1">If a user with the explicit UID given by <tt>-u</tt> already
      exists, you can override the error message with this option, to create
      non-unique UIDs.</td>
  </tr>
  <tr>
    <td class="t2">-p password</td>
    <td class="t2">Change the users password to the one specified.
      <tt>password</tt> needs to be in encrypted form.</td>
  </tr>
  <tr>
    <td class="t1">-s shell</td>
    <td class="t1">Changes the user's default shell</td>
  </tr>
  <tr>
    <td class="t2">-u uid</td>
    <td class="t2">Changes the user's identification number. It must be unique,
      unless <tt>-o</tt> is given.</td>
  </tr>
</table>
</div>

<p class="block"><tt>usermod</tt> updates the user's account with the
information given on the command line. Not specified options will not change
the corresponding account field. If <tt>-I</tt> is passed, the user is
interactively asked for what parameters shall be changed, by presenting a
default value, which he may either accept (by leaving the answer empty) or
specifying a new value. The <tt>-A</tt>, <tt>-B</tt>, <tt>-I</tt>, and
<tt>-M</tt> options are unique to Vitalnix. The configuration file for
<tt>usermod</tt> is <tt>sysprog/../vetc/usermod.conf</tt>, or
<tt>/etc/usermod.conf</tt>.</p>

<h1>userdel - Delete a user account <img src="d_arr.png" /></h1>

<div class="pleft">
<table>
  <tr>
    <td class="cmd"><tt><b>vuserdel</b> [-A [command]] [-B [command]] [-M
      backend] [-r] login</tt></td>
  </tr>
</table>
</div>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1">-A [command]</td>
    <td class="t1">Runs the specified command after the user was successfully
      added. If <tt>-A</tt> is given, but with a zero-length command string,
      the default command in the configuration file is not run.</td>
  </tr>
  <tr>
    <td class="t2">-B [command]</td>
    <td class="t2">Runs the specified command before the user is added. It is
      run always, even if adding fails. If <tt>-B</tt> is given, but with a
      zero-length command string, the default command in the configuration file
      is not run.</td>
  </tr>
  <tr>
    <td class="t1">-M backend</td>
    <td class="t1">Uses the specified backend rather than the default one
      listed in the ACCDB configuration file</td>
  </tr>
  <tr>
    <td class="t2">-r</td>
    <td class="t2">Removes the user's home directory including files located
      therein and its mail spool (assuming <tt>/var/spool/mail/</tt>).</td>
  </tr>
</table>
</div>

<p class="block"><tt>userdel</tt> deletes the specified user. The <tt>-A</tt>,
<tt>-B</tt> and <tt>-M</tt> options are unique to Vitalnix. The configuration
file for <tt>userdel</tt> is <tt>sysprog/../vetc/userdel.conf</tt>, or
<tt>/etc/userdel.conf</tt>.

<h1>groupadd - Create a new group <img src="d_arr.png" /></h1>

<div class="pleft">
<table>
  <tr>
    <td class="cmd"><tt><b>vgroupadd</b> [-A [command]] [-B [command]] [-I] [-M
      backend] [-g gid [-o]|-r] group</tt></td>
  </tr>
</table>
</div>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1">-A [command]</td>
    <td class="t1">Runs the specified command after the user was successfully
      added. If <tt>-A</tt> is given, but with a zero-length command string,
      the default command in the configuration file is not run.</td>
  </tr>
  <tr>
    <td class="t2">-B [command]</td>
    <td class="t2">Runs the specified command before the user is added. It is
      run always, even if adding fails. If <tt>-B</tt> is given, but with a
      zero-length command string, the default command in the configuration file
      is not run.</td>
  </tr>
  <tr>
    <td class="t1">-I</td>
    <td class="t1">Interactively prompt for the new group's parameters</td>
  </tr>
  <tr>
    <td class="t2">-M backend</td>
    <td class="t2">Uses the specified backend rather than the default one
      listed in the ACCDB configuration file</td>
  </tr>
  <tr>
    <td class="t1">-g gid</td>
    <td class="t1">The numerical group identification number of the new group.
      The default is to assign the user the next greatest GID available. This
      option overrides <tt>-r</tt>.</td>
  </tr>
  <tr>
    <td class="t2">-o</td>
    <td class="t2">If a group with the explicit GID given by <tt>-g</tt>
      already exists, you can override the error message with this option, to
      create non-unique GIDs.</td>
  </tr>
  <tr>
    <td class="t1">-r</td>
    <td class="t1">Create a system group, that is, with a lower GID than the
      hardcoded default (100), or what is specified for <tt>MIN_GID</tt> in the
      autouid configuration file. The <tt>-g</tt> option overrides this
      one.</td>
  </tr>
</table>
</div>

<p class="block"><tt>groupadd</tt> will create a new group using the supplied
parameters, and, if <tt>-I</tt> is given, interactively asks the user for the
parameters.
In interactive mode, each specified option is printed with a default value in
square brackets. If an empty string is entered, the default value is taken. The
<tt>-A</tt>, <tt>-B</tt>, <tt>-I</tt>, and <tt>-M</tt> options are unique to
Vitalnix. The configuration file for <tt>groupadd</tt> is
<tt>sysprog/../vetc/groupadd.conf</tt>, or <tt>/etc/groupadd.conf</tt>.</p>

<h1>groupmod - Modify a group <img src="d_arr.png" /></h1>

<div class="pleft">
<table>
  <tr>
    <td class="cmd"><tt><b>vgroupmod</b> [-A [command]] [-B [command]] [-I] [-M
      backend] [-g gid [-o]] [-n group_name] group</tt></td>
  </tr>
</table>
</div>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1">-A [command]</td>
    <td class="t1">Runs the specified command after the user was successfully
      added. If <tt>-A</tt> is given, but with a zero-length command string,
      the default command in the configuration file is not run.</td>
  </tr>
  <tr>
    <td class="t2">-B [command]</td>
    <td class="t2">Runs the specified command before the user is added. It is
      run always, even if adding fails. If <tt>-B</tt> is given, but with a
      zero-length command string, the default command in the configuration file
      is not run.</td>
  </tr>
  <tr>
    <td class="t1">-I</td>
    <td class="t1">Interactively prompt for the new group's parameters</td>
  </tr>
  <tr>
    <td class="t2">-M backend</td>
    <td class="t2">Uses the specified backend rather than the default one
      listed in the ACCDB configuration file</td>
  </tr>
  <tr>
    <td class="t1">-g gid</td>
    <td class="t1">Changes the user's identification number. It must be unique,
      unless <tt>-o</tt> is given.</td>
  </tr>
  <tr>
    <td class="t2">-o</td>
    <td class="t2">If a group with the explicit GID given by <tt>-g</tt>
      already exists, you can override the error message with this option, to
      create non-unique GIDs.</td>
  </tr>
  <tr>
    <td class="t1">-n name</td>
    <td class="t1">Changes the group's name, which must be unique.</td>
  </tr>
</table>
</div>

<p class="block"><tt>groupmod</tt> updates the group's info with the data given
on the command line. Not specified options will not change the corresponding
fields in the database. If <tt>-I</tt> is passed, the user is interactively
asked for what parameters shall be changed, by presenting a default value,
which he may either accept (by leaving the answer empty) or specifying a new
value. The <tt>-A</tt>, <tt>-B</tt>, <tt>-I</tt>, and <tt>-M</tt> options are
unique to Vitalnix. The configuration file for <tt>groupmod</tt> is
<tt>sysprog/../vetc/groupmod.conf</tt>, or <tt>/etc/groupmod.conf</tt>.</p>

<h1>groupdel - Delete a group <img src="d_arr.png" /></h1>

<div class="pleft">
<table>
  <tr>
    <td class="cmd"><tt><b>vgroupdel</b> [-A [command]] [-B [command]] [-F] [-M
      backend] group</tt></td>
  </tr>
</table>
</div>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1">-A [command]</td>
    <td class="t1">Runs the specified command after the user was successfully
      added. If <tt>-A</tt> is given, but with a zero-length command string,
      the default command in the configuration file is not run.</td>
  </tr>
  <tr>
    <td class="t2">-B [command]</td>
    <td class="t2">Runs the specified command before the user is added. It is
      run always, even if adding fails. If <tt>-B</tt> is given, but with a
      zero-length command string, the default command in the configuration file
      is not run.</td>
  </tr>
  <tr>
    <td class="t1">-F</td>
    <td class="t1"><tt>userdel</tt> will, by default, not delete any groups, of
      which a user is a primary member of. <tt>-F</tt> forces to delete the
      group.</td>
  </tr>
  <tr>
    <td class="t2">-I</td>
    <td class="t2">Interactively prompt for the new group's parameters</td>
  </tr>
  <tr>
    <td class="t1">-M backend</td>
    <td class="t1">Uses the specified backend rather than the default one
      listed in the ACCDB configuration file</td>
  </tr>
</table>
</div>

<p class="block"><tt>groupdel</tt> deletes the specified group, but will take
care for whether any use has that group as a primary group set. All options
provided are unique to Vitalnix. The configuration file for <tt>groupdel</tt>
is <tt>sysprog/../vetc/groupdel.conf</tt>, or <tt>/etc/groupdel.conf</tt>.

<?php include_once("zfooter.php"); ?>
