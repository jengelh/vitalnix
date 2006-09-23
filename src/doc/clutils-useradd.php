<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>useradd&nbsp;-- Create a new user</p>

<h1>Usage</h1>

<p class="code"><tt><b>vxuseradd</b> <b>[</b>-A <b>[</b><i>command</i><b>]]</b>
<b>[</b>-B <b>[</b><i>command</i><b>]]</b> <b>[</b>-I<b>]</b> <b>[</b>-M
<i>backend</i><b>]</b> <b>[</b>-S<b>]</b> <b>[</b>-G
<i>group</i><b>[</b>,<b>...]]</b> <b>[</b>-c <i>comment</i><b>]</b> <b>[</b>-d
<i>home_dir</i><b>]</b> <b>[</b>-e <i>date</i><b>]</b> <b>[</b>-f
<i>days</i><b>]</b> <b>[</b>-g <i>group</i><b>]</b> <b>[</b>-m <b>[</b>-k
<i>skel_dir</i><b>]]</b> <b>[</b>-p <i>passwd</i><b>]</b> <b>[</b>-s
<i>shell</i><b>]</b> <b>[</b>-u <i>uid</i> <b>[</b>-o<b>]|</b>-r<b>]</b>
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
    <td class="t1">-I</td>
    <td class="t1">Interactively prompt for the new user's parameters</td>
  </tr>
  <tr>
    <td class="t2">-M <i>backend</i></td>
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
    <td class="t2">-G <i>group</i><b>[</b>,<b>...]</b></td>
    <td class="t2">The supplemental (secondary) groups the user is a member in,
      separated by comma. <tt>group</tt> can either be the group's name or its
      GID. <b><i>(UNSUPPORTED)</i></b></td>
  </tr>
  <tr>
    <td class="t1">-c <i>comment</i></td>
    <td class="t1">The comment field for the new user. It can be in turn again
      split up in fields, separated by comma. Usually, the real name of the
      user is stored in the first field, the others are left for other
      commentary or data.</td>
  </tr>
  <tr>
    <td class="t2">-d <i>dir</i></td>
    <td class="t2">The new user will get <tt>dir</tt> as his home directory.
      This option alone does not create his home directory, but rather only
      writes that directory information into the user database. You usually
      want to pass <tt>-m</tt> also along to create the directory. This option
      overrides <tt>-S</tt>.</td>
  </tr>
  <tr>
    <td class="t1">-e <i>date</i></td>
    <td class="t1">Date on which the account expires. Date formats must be
      YYYY-MM-DD. The default is to not let the account expire.</td>
  </tr>
  <tr>
    <td class="t2">-f <i>days</i></td>
    <td class="t2">The number of days which need to pass after a password
      expiration (that is not <tt>-e</tt>!) until the account is locked
      down.</td>
  </tr>
  <tr>
    <td class="t1">-g <i>group</i></td>
    <td class="t1">The group name or number of the user's initial (primary)
      group. <tt>group</tt> can either be the group's name or its GID.</td>
  </tr>
  <tr>
    <td class="t2">-k <i>dir</i></td>
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
    <td class="t1">-p <i>pass</i></td>
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
    <td class="t1">-s <i>shell</i></td>
    <td class="t1">Use <tt>shell</tt> as the default command interpreter for
      the new user. The default is <tt>/bin/bash</tt> or whatever you specified
      as <tt>SHELL</tt> in the configuration file.</td>
  </tr>
  <tr>
    <td class="t2">-u <i>uid</i></td>
    <td class="t2">The numerical user id of the new user. The default is to
      assign the user the next greatest UID. This option overrides
      <tt>-r</tt>.</td>
  </tr>
</table>

<h1>Description</h1>

<p class="block"><tt>useradd</tt> will create a new user using the supplied
parameters, and, if <tt>-I</tt> is given, interactively asks the user for the
parameters.</p>

<p class="block">In interactive mode, each specified option is printed with a
default value in square brackets. If an empty string is entered, the default
value is taken. The <tt>-A</tt>, <tt>-B</tt>, <tt>-I</tt>, <tt>-M</tt> and
<tt>-S</tt> options are unique to Vitalnix. Shadow's <tt>-D</tt> option is not
provided. The configuration file for <tt>useradd</tt> is
<tt>/etc/vitalnix/useradd.conf</tt>.</p>

<?php include_once("Base-footer.php"); ?>
