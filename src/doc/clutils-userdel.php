<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxuserdel&nbsp;-- Delete a user account</p>

<h1>Usage</h1>

<p class="code"><tt><b>vxuserdel</b> <b>[</b>-A <b>[</b><i>command</i><b>]]</b>
<b>[</b>-B <b>[</b><i>command</i><b>]]</b> <b>[</b>-F<b>]</b> <b>[</b>-M
<i>database</i><b>]</b> <b>[</b>-r<b>]</b> <i>login</i></tt></p

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
    <td class="t1">-F</td>
    <td class="t1">Force deletion even if UID is zero or the username is
      <i>root</i></td>
  <tr>
    <td class="t2">-M <i>database</i></td>
    <td class="t2">Uses the specified database rather than the default one
      listed in the VXPDB configuration file</td>
  </tr>
  <tr>
    <td class="t1">-r</td>
    <td class="t1">Removes the user's home directory including files located
      therein and its mail spool (assuming <tt>/var/spool/mail/</tt>).</td>
  </tr>
</table>

<h1>Description</h1>

<p class="block"><tt>userdel</tt> deletes the specified user. The <tt>-A</tt>,
<tt>-B</tt> and <tt>-M</tt> options are unique to Vitalnix. The configuration
file for <tt>userdel</tt> is <tt>/etc/vitalnix/userdel.conf</tt>.</p>

<?php include_once("Base-footer.php"); ?>
