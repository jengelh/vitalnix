<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>groupadd&nbsp;-- Create a new group</p>

<h1>Usage</h1>

<p class="code"><tt><b>vxgroupadd</b> <b>[</b>-A
<b>[</b><i>command</i><b>]]</b> <b>[</b>-B <b>[</b><i>command</i><b>]]</b>
<b>[</b>-I<b>]</b> <b>[</b>-M <i>backend</i><b>]</b> <b>[</b>-g <i>gid</i>
<b>[</b>-o<b>]|</b>-r<b>]</b> <i>group</i></tt></p>

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
    <td class="t1">Interactively prompt for the new group's parameters</td>
  </tr>
  <tr>
    <td class="t2">-M <i>backend</i></td>
    <td class="t2">Uses the specified backend rather than the default one
      listed in the ACCDB configuration file</td>
  </tr>
  <tr>
    <td class="t1">-g <i>gid</i></td>
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

<h1>Description</h1>

<p class="block"><tt>groupadd</tt> will create a new group using the supplied
parameters, and, if <tt>-I</tt> is given, interactively asks the user for the
parameters.</p>

<p class="block">In interactive mode, each specified option is printed with a
default value in square brackets. If an empty string is entered, the default
value is taken. The <tt>-A</tt>, <tt>-B</tt>, <tt>-I</tt>, and <tt>-M</tt>
options are unique to Vitalnix. The configuration file for <tt>groupadd</tt> is
<tt>/etc/vitalnix/groupadd.conf</tt>.</p>

<?php include_once("Base-footer.php"); ?>
