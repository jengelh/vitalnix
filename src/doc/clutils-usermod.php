<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p class="block">usermod&nbsp;-- Modify a user account</p>

<h1>Usage</h1>

<p class="code"><tt><b>vxusermod</b> <b>[</b>-A <b>[</b><i>command</i><b>]]</b>
<b>[</b>-B <b>[</b><i>command</i><b>]]</b> <b>[</b>-G
<i>group</i>,<b>[...]]</b> <b>[</b>-L<b>]</b> <b>[</b>-M
<i>database</i><b>]</b> <b>[</b>-U<b>]</b> <b>[</b>-c <i>comment</i><b>]</b>
<b>[</b>-d <i>home_dir</i> <b>[</b>-m<b>]]</b> <b>[</b>-e <i>date</i><b>]</b>
<b>[</b>-f <i>inactive_days</i><b>]</b> <b>[</b>-g <i>primary_group</i><b>]</b>
<b>[</b>-l <i>login_name</i><b>]</b> <b>[</b>-p <i>passwd</i><b>]</b>
<b>[</b>-s <i>shell</i><b>]</b> <b>[</b>-u <i>uid</i> <b>[</b>-o<b>]]</b>
<i>login</i></tt></p>

<h1>Options</h1>

<table border="1" class="bordered">
  <tr>
    <td class="t1">-A <b>[</b><i>command</i><b>]</b></td>
    <td class="t1">Runs the specified command after the user was successfully
      added. If <tt>-A</tt> is given, but with a zero-length command string,
      the default command in the configuration file is not run.</td>
  </tr>
  <tr>
    <td class="t2">-B <b>[</b><i>command</i><b>]</b></td>
    <td class="t2">Runs the specified command before the user is added. It is
      run always, even if adding fails. If <tt>-B</tt> is given, but with a
      zero-length command string, the default command in the configuration file
      is not run.</td>
  </tr>
  <tr>
    <td class="t1">-G <i>group</i><b>[</b>,<b>...]</b></td>
    <td class="t1">Changes the user's info about which supplemental (secondary)
      groups he belongs to. Note that this will cancel his membership in any of
      the groups not specified, so you to add a user to another group, you need
      to run <tt>usermod</tt> with <tt>-G first</tt> and the second time with
      <tt>-G first,second</tt>. <b><i>(UNSUPPORTED)</i></b></td>
  </tr>
  <tr>
    <td class="t2">-I</td>
    <td class="t2">Interactively prompt for the new parameters</td>
  </tr>
  <tr>
    <td class="t1">-L</td>
    <td class="t1">Lock the user's account (prohibit logins)
      <b><i>(UNSUPPORTED)</i></b></td>
  </tr>
  <tr>
    <td class="t2">-M <i>database</i></td>
    <td class="t2">Uses the specified database rather than the default one
      listed in the VXPDB configuration file</td>
  </tr>
  <tr>
    <td class="t1">-U</td>
    <td class="t1">Unlock the user's account <b><i>(UNSUPPORTED)</i></b></td>
  </tr>
  <tr>
    <td class="t2">-c <i>comment</i></td>
    <td class="t2">Changes the comment (GECOS) field</td>
  </tr>
  <tr>
    <td class="t1">-d <i>home</i></td>
    <td class="t1">Changes the home directory as found in the user database.
      Also see <tt>-m</tt>.</td>
  </tr>
  <tr>
    <td class="t2">-e <i>date</i></td>
    <td class="t2">The new date on which the user account will expire. (Valid
      format is YYYY-MM-DD.)</td>
  </tr>
  <tr>
    <td class="t1">-f <i>days</i></td>
    <td class="t1">The number of days which need to pass after a password
      expiration (that is not <tt>-e</tt>!) until the account is locked
      down.</td>
  </tr>
  <tr>
    <td class="t2">-g <i>group</i></td>
    <td class="t2">Changes the users initial (primary) group</td>
  </tr>
  <tr>
    <td class="t1">-l <i>login</i></td>
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
    <td class="t2">-p <i>password</i></td>
    <td class="t2">Change the users password to the one specified.
      <tt>password</tt> needs to be in encrypted form.</td>
  </tr>
  <tr>
    <td class="t1">-s <i>shell</i></td>
    <td class="t1">Changes the user's default shell</td>
  </tr>
  <tr>
    <td class="t2">-u <i>uid</i></td>
    <td class="t2">Changes the user's identification number. It must be unique,
      unless <tt>-o</tt> is given.</td>
  </tr>
</table>

<h1>Description</h1>

<p class="block"><tt>usermod</tt> updates the user's account with the
information given on the command line. Not specified options will not change
the corresponding account field.</p>

<p class="block">If <tt>-I</tt> is passed, the user is interactively asked for
what parameters shall be changed, by presenting a default value, which he may
either accept (by leaving the answer empty) or specifying a new value. The
<tt>-A</tt>, <tt>-B</tt>, <tt>-I</tt>, and <tt>-M</tt> options are unique to
Vitalnix. The configuration file for <tt>usermod</tt> is
<tt>/etc/vitalnix/usermod.conf</tt>.</p>

<?php include_once("Base-footer.php"); ?>
