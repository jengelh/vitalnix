<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>groupmod&nbsp;-- Modify a group</p>

<h1>Usage</h1>

<p class="code"><code><b>vxgroupmod</b> <b>[</b>-A
<b>[</b><i>command</i><b>]]</b> <b>[</b>-B <b>[</b><i>command</i><b>]]</b>
<b>[</b>-M <i>database</i><b>]</b> <b>[</b>-g <i>gid</i> <b>[</b>-o<b>]]</b>
<b>[</b>-n <i>group_name</i><b>]</b> <i>group</i></code></p>

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
    <td class="t1n"><code>-M <i>database</i></code></td>
    <td class="t1">Uses the specified database rather than the default one
      listed in the VXDB configuration file</td>
  </tr>
  <tr>
    <td class="t2n"><code>-g <i>gid</i></code></td>
    <td class="t2">Changes the user's identification number. It must be unique,
      unless <code>-o</code> is given.</td>
  </tr>
  <tr>
    <td class="t1n"><code>-o</code></td>
    <td class="t1">If a group with the explicit GID given by <code>-g</code>
      already exists, you can override the error message with this option, to
      create non-unique GIDs.</td>
  </tr>
  <tr>
    <td class="t2n"><code>-n name</code></td>
    <td class="t2">Changes the group's name, which must be unique.</td>
  </tr>
</table>

<h1>Description</h1>

<p class="block"><i>groupmod</i> updates the group's info with the data given
on the command line. Not specified options will not change the corresponding
fields in the database. The configuration file for <code>groupmod</code> is
<code>/etc/vitalnix/groupmod.conf</code>.</p>

<?php include_once("Base-footer.php"); ?>
