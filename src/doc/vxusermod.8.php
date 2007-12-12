<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p class="block">usermod&nbsp;-- Modify a user account</p>

<h1>Usage</h1>

<p class="code"><code><b>vxusermod</b> <b>[</b>-A
<b>[</b><i>command</i><b>]]</b> <b>[</b>-B <b>[</b><i>command</i><b>]]</b>
<b>[</b>-G <i>group</i>,<b>[...]]</b> <b>[</b>-L<b>]</b> <b>[</b>-M
<i>database</i><b>]</b> <b>[</b>-U<b>]</b> <b>[</b>-c <i>comment</i><b>]</b>
<b>[</b>-d <i>home_dir</i> <b>[</b>-m<b>]]</b> <b>[</b>-e <i>date</i><b>]</b>
<b>[</b>-f <i>inactive_days</i><b>]</b> <b>[</b>-g <i>primary_group</i><b>]</b>
<b>[</b>-l <i>login_name</i><b>]</b> <b>[</b>-p <i>passwd</i><b>]</b>
<b>[</b>-s <i>shell</i><b>]</b> <b>[</b>-u <i>uid</i> <b>[</b>-o<b>]]</b>
<i>login</i></code></p>

<h1>Options</h1>

<table border="1">
  <tr>
    <td class="t1n"><code>-A <b>[</b><i>command</i><b>]</b></code></td>
    <td class="t1">Runs the specified command after the user was successfully
      added. If <code>-A</code> is given, but with a zero-length command
      string, the default command in the configuration file is not run.</td>
  </tr>
  <tr>
    <td class="t2n"><code>-B <b>[</b><i>command</i><b>]</b></code></td>
    <td class="t2">Runs the specified command before the user is added. It is
      run always, even if adding fails. If <code>-B</code> is given, but with a
      zero-length command string, the default command in the configuration file
      is not run.</td>
  </tr>
  <tr>
    <td class="t1n"><code>-G <i>group</i><b>[</b>,<b>...]</b></code></td>
    <td class="t1">Changes the user's info about which supplemental (secondary)
      groups he belongs to. Note that this will cancel his membership in any of
      the groups not specified, so you to add a user to another group, you need
      to run <code>usermod</code> with <code>-G first</code> and the second
      time with <code>-G first,second</code>. <b><i>(UNSUPPORTED)</i></b></td>
  </tr>
  <tr>
    <td class="t2n"><code>-I</code></td>
    <td class="t2">Interactively prompt for the new parameters</td>
  </tr>
  <tr>
    <td class="t1n"><code>-L</code></td>
    <td class="t1">Lock the user's account (prohibit logins)
      <b><i>(UNSUPPORTED)</i></b></td>
  </tr>
  <tr>
    <td class="t2n"><code>-M <i>database</i></code></td>
    <td class="t2">Uses the specified database rather than the default one
      listed in the VXDB configuration file</td>
  </tr>
  <tr>
    <td class="t1n"><code>-U</code></td>
    <td class="t1">Unlock the user's account <b><i>(UNSUPPORTED)</i></b></td>
  </tr>
  <tr>
    <td class="t2n"><code>-c <i>comment</i></code></td>
    <td class="t2">Changes the comment (GECOS) field</td>
  </tr>
  <tr>
    <td class="t1n"><code>-d <i>home</i></code></td>
    <td class="t1">Changes the home directory as found in the user database. 
      Also see <code>-m</code>.</td>
  </tr>
  <tr>
    <td class="t2n"><code>-e <i>date</i></code></td>
    <td class="t2">The new date on which the user account will expire. (Valid
      format is YYYY-MM-DD.)</td>
  </tr>
  <tr>
    <td class="t1n"><code>-f <i>days</i></code></td>
    <td class="t1">The number of days which need to pass after a password
      expiration (which is not <code>-e</code>!) until the account is locked
      down.</td>
  </tr>
  <tr>
    <td class="t2n"><code>-g <i>group</i></code></td>
    <td class="t2">Changes the users initial (primary) group</td>
  </tr>
  <tr>
    <td class="t1n"><code>-l <i>login</i></code></td>
    <td class="t1">Changes the login name for the user, which must be
      unique.</td>
  </tr>
  <tr>
    <td class="t2n"><code>-m</code></td>
    <td class="t2">When <code>-d</code> is given, specifying <code>-m</code>
      will move data from the old home directory into the new one, necessarily
      creating the new directory if it does not exist yet. 
      <b><i>(UNSUPPORTED)</i></b></td>
  </tr>
  <tr>
    <td class="t1n"><code>-o</code></td>
    <td class="t1">If a user with the explicit UID given by <code>-u</code>
      already exists, you can override the error message with this option, to
      create non-unique UIDs.</td>
  </tr>
  <tr>
    <td class="t2n"><code>-p <i>password</i></code></td>
    <td class="t2">Change the users password to the one specified.
      <code>password</code> needs to be in encrypted form.</td>
  </tr>
  <tr>
    <td class="t1n"><code>-s <i>shell</i></code></td>
    <td class="t1">Changes the user's default shell</td>
  </tr>
  <tr>
    <td class="t2n"><code>-u <i>uid</i></code></td>
    <td class="t2">Changes the user's identification number. It must be unique,
      unless <code>-o</code> is given.</td>
  </tr>
</table>

<h1>Description</h1>

<p class="block"><i>usermod</i> updates the user's account with the information
given on the command line. Not specified options will not change the
corresponding account field.</p>

<p class="block">If <code>-I</code> is passed, the user is interactively asked
for what parameters shall be changed, by presenting a default value, which he
may either accept (by leaving the answer empty) or specifying a new value. The
<code>-A</code>, <code>-B</code>, <code>-I</code>, and <code>-M</code> options
are unique to Vitalnix. The configuration file for <code>usermod</code> is
<code>/etc/vitalnix/usermod.conf</code>.</p>

<?php include_once("Base-footer.php"); ?>
