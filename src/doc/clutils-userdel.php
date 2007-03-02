<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxuserdel&nbsp;-- Delete a user account</p>

<h1>Usage</h1>

<p class="code"><code><b>vxuserdel</b> <b>[</b>-A
<b>[</b><i>command</i><b>]]</b> <b>[</b>-B <b>[</b><i>command</i><b>]]</b>
<b>[</b>-F<b>]</b> <b>[</b>-M <i>database</i><b>]</b> <b>[</b>-r<b>]</b>
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
    <td class="t1n"><code>-F</code></td>
    <td class="t1">Force deletion even if UID is zero or the username is
      <i>root</i></td>
  </tr>
  <tr>
    <td class="t2n"><code>-M <i>database</i></code></td>
    <td class="t2">Uses the specified database rather than the default one
      listed in the VXPDB configuration file</td>
  </tr>
  <tr>
    <td class="t1n"><code>-r</code></td>
    <td class="t1">Removes the user's home directory including files located
      therein and its mail spool (assuming <code>/var/spool/mail/</code>).</td>
  </tr>
</table>

<h1>Description</h1>

<p class="block"><i>userdel</i> deletes the specified user. The
<code>-A</code>, <code>-B</code> and <code>-M</code> options are unique to
Vitalnix. The configuration file for <code>userdel</code> is
<code>/etc/vitalnix/userdel.conf</code>.</p>

<?php include_once("Base-footer.php"); ?>
