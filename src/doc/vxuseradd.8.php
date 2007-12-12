<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>useradd&nbsp;-- Create a new user</p>

<h1>Usage</h1>

<p class="code"><code><b>vxuseradd</b> <b>[</b>-A
<b>[</b><i>command</i><b>]]</b> <b>[</b>-B <b>[</b><i>command</i><b>]]</b>
<b>[</b>-G <i>group</i><b>[</b>,<b>...]]</b> <b>[</b>-I<b>]</b> <b>[</b>-M
<i>database</i><b>]</b> <b>[</b>-S<b>]</b> <b>[</b>-c <i>comment</i><b>]</b>
<b>[</b>-d <i>home_dir</i><b>]</b> <b>[</b>-e <i>date</i><b>]</b> <b>[</b>-f
<i>days</i><b>]</b> <b>[</b>-g <i>group</i><b>]</b> <b>[</b>-m <b>[</b>-k
<i>skel_dir</i><b>]]</b> <b>[</b>-p <i>passwd</i><b>]</b> <b>[</b>-s
<i>shell</i><b>]</b> <b>[</b>-u <i>uid</i> <b>[</b>-o<b>]|</b>-r<b>]</b>
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
    <td class="t1">The supplemental (secondary) groups the user is a member in,
      separated by comma. <code>group</code> can either be the group's name or
      its GID. <b><i>(UNSUPPORTED)</i></b></td>
  </tr>
  <tr>
    <td class="t2n"><code>-I</code></td>
    <td class="t2">Interactively prompt for the new user's parameters</td>
  </tr>
  <tr>
    <td class="t1n"><code>-M <i>database</i></code></td>
    <td class="t1">Uses the specified database rather than the default one
      listed in the VXDB configuration file</td>
  </tr>
  <tr>
    <td class="t2n"><code>-S</code></td>
    <td class="t2">Uses the "split home" feature. This will create the home
      directory as <code>/home/u/username</code> rather than the default
      <code>/home/username</code>. Specifying <code>-S</code> twice (which is
      the maximum) will result in a two-level split, i.e. 
      <code>/home/u/us/username</code>.  The <code>-d</code> option overrides
      this one.</td>
  </tr>
  <tr>
    <td class="t1n"><code>-c <i>comment</i></code></td>
    <td class="t1">The comment field for the new user. It can be in turn again
      split up in fields, separated by comma. Usually, the real name of the
      user is stored in the first field, the others are left for other
      commentary or data.</td>
  </tr>
  <tr>
    <td class="t2n"><code>-d <i>dir</i></code></td>
    <td class="t2">The new user will get <code>dir</code> as his home
      directory.  This option alone does not create his home directory, but
      rather only writes that directory information into the user database. You
      usually want to pass <code>-m</code> also along to create the directory.
      This option overrides <code>-S</code>.</td>
  </tr>
  <tr>
    <td class="t1n"><code>-e <i>date</i></code></td>
    <td class="t1">Date on which the account expires. Date formats must be
      YYYY-MM-DD. The default is to not let the account expire.</td>
  </tr>
  <tr>
    <td class="t2n"><code>-f <i>days</i></code></td>
    <td class="t2">The number of days which need to pass after a password
      expiration (that is not <code>-e</code>!) until the account is locked
      down.</td>
  </tr>
  <tr>
    <td class="t1n"><code>-g <i>group</i></code></td>
    <td class="t1">The group name or number of the user's initial (primary)
      group. <code>group</code> can either be the group's name or its GID.</td>
  </tr>
  <tr>
    <td class="t2n"><code>-k <i>dir</i></code></td>
    <td class="t2">The skeleton directory to use for the new user. Contents
      from this directory will be copied into the user's.</td>
  </tr>
  <tr>
    <td class="t1n"><code>-m</code></td>
    <td class="t1">Create the user's home directory. The default is to create a
      directory called the same name as the user in <code>/home</code>, or what
      you specified as <code>HOME</code> in the configuration file, or if you
      passed <code>-S</code> or <code>-d</code>. If this option is given, an
      implicit <code>-k /etc/skel</code> (or <code>SKEL</code>) takes effect,
      unless an explicit <code>-k</code> is specified.</td>
  </tr>
  <tr>
    <td class="t2n"><code>-o</code></td>
    <td class="t2">If a user with the explicit UID given by <code>-u</code>
      already exists, you can override the error message with this option, to
      create non-unique UIDs.</td>
  </tr>
  <tr>
    <td class="t1n"><code>-p <i>pass</i></code></td>
    <td class="t1">The password to use for the new user. It must already be
      encrypted when passed with <code>-p</code>.</td>
  </tr>
  <tr>
    <td class="t2n"><code>-r</code></td>
    <td class="t2">Create a system user, that is, with a lower UID than the
      hardcoded default (100), or what is specified for <code>MIN_UID</code> in
      the autouid configuration file. The <code>-u</code> option overrides this
      one.</td>
  </tr>
  <tr>
    <td class="t1n"><code>-s <i>shell</i></code></td>
    <td class="t1">Use <code>shell</code> as the default command interpreter
      for the new user. The default is <code>/bin/bash</code> or whatever you
      specified as <code>SHELL</code> in the configuration file.</td>
  </tr>
  <tr>
    <td class="t2n"><code>-u <i>uid</i></code></td>
    <td class="t2">The numerical user id of the new user. The default is to
      assign the user the next greatest UID. This option overrides
      <code>-r</code>.</td>
  </tr>
</table>

<h1>Description</h1>

<p class="block"><i>useradd</i> will create a new user using the supplied
parameters, and, if <code>-I</code> is given, interactively asks the user for
the parameters.</p>

<p class="block">In interactive mode, each specified option is printed with a
default value in square brackets. If an empty string is entered, the default
value is taken. The <code>-A</code>, <code>-B</code>, <code>-I</code>,
<code>-M</code> and <code>-S</code> options are unique to Vitalnix. Shadow's
<code>-D</code> option (LDAP binddn) is not provided. The configuration file
for <code>useradd</code> is <code>/etc/vitalnix/useradd.conf</code>.</p>

<?php include_once("Base-footer.php"); ?>
