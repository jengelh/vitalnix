<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>groupadd&nbsp;-- Create a new group</p>

<h1>Usage</h1>

<p class="code"><code><b>vxgroupadd</b> <b>[</b>-A
<b>[</b><i>command</i><b>]]</b> <b>[</b>-B <b>[</b><i>command</i><b>]]</b>
<b>[</b>-M <i>database</i><b>]</b> <b>[</b>-g <i>gid</i>
<b>[</b>-o<b>]|</b>-r<b>]</b> <i>group</i></code></p>

<h1>Options</h1>

<table border="1">
  <tr>
    <td class="t1">-A <b>[</b><i>command</i><b>]</b></td>
    <td class="t1">Runs the specified command after the user was successfully
      added. If <code>-A</code> is given, but with a zero-length command
      string, the default command in the configuration file is not run.</td>
  </tr>
  <tr>
    <td class="t2">-B <b>[</b><i>command</i><b>]</b></td>
    <td class="t2">Runs the specified command before the user is added. It is
      run always, even if adding fails. If <code>-B</code> is given, but with a
      zero-length command string, the default command in the configuration file
      is not run.</td>
  </tr>
  <tr>
    <td class="t1">-M <i>database</i></td>
    <td class="t1">Uses the specified database rather than the default one
      listed in the VXPDB configuration file</td>
  </tr>
  <tr>
    <td class="t2">-g <i>gid</i></td>
    <td class="t2">The numerical group identification number of the new group. 
      The default is to assign the user the next greatest GID available. This
      option overrides <code>-r</code>.</td>
  </tr>
  <tr>
    <td class="t1">-o</td>
    <td class="t1">If a group with the explicit GID given by <code>-g</code>
      already exists, you can override the error message with this option, to
      create non-unique GIDs.</td>
  </tr>
  <tr>
    <td class="t2">-r</td>
    <td class="t2">Create a system group, that is, with a lower GID than the
      hardcoded default (100), or what is specified for <code>MIN_GID</code> in
      the autouid configuration file. The <code>-g</code> option overrides this
      one.</td>
  </tr>
</table>

<h1>Description</h1>

<p class="block"><i>groupadd</i> will create a new group using the supplied
parameters. The configuration file for <code>groupadd</code> is
<code>/etc/vitalnix/groupadd.conf</code>.</p>

<?php include_once("Base-footer.php"); ?>
