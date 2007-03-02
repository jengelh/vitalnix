<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>groupdel&nbsp;-- Delete a group</p>

<h1>Usage</h1>

<p class="code"><code><b>vxgroupdel</b> <b>[</b>-A
<b>[</b><i>command</i><b>]]</b> <b>[</b>-B <b>[</b><i>command</i><b>]]</b>
<b>[</b>-F<b>]</b> <b>[</b>-M <i>database</i><b>]</b> <i>group</i></code></p>

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
    <td class="t1n"><code>-F</code></td>
    <td class="t1"><code>userdel</code> will, by default, not delete any
      groups, of which a user is a primary member of. <code>-F</code> forces to
      delete the group.</td>
  </tr>
  <tr>
    <td class="t2n"><code>-M <i>database</i></code></td>
    <td class="t2">Uses the specified database rather than the default one
      listed in the VXPDB configuration file</td>
  </tr>
</table>

<h1>Description</h1>

<p class="block"><i>groupdel</i> deletes the specified group, but will take
care for whether any use has that group as a primary group set. All options
provided are unique to Vitalnix. The configuration file for
<code>groupdel</code> is <code>/etc/vitalnix/groupdel.conf</code>.</p>

<?php include_once("Base-footer.php"); ?>
