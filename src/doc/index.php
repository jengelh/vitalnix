<?php include_once("Base-header.php"); ?>

<table>
<tr>
<td valign="top">

<h1>Manpages</h1>

<ul>
  <li><a href="gen-intro.php">Introduction</a></li>
  <li><a href="gen-tech.php">Technical aspects</a></li>
  <li>Shell utilities (clutils)<br />
    <ul>
      <li><a href="clutils-mdfixuuid.php">mdfixuuid</a></li>
      <li><a href="clutils-mdpwlfmt.php">mdpwlfmt</a></li>
      <li><a href="clutils-mdsingle.php">mdsingle</a></li>
      <li><a href="clutils-mdsync.php">mdsync</a></li>
      <li><a href="clutils-pdbdump.php">pdbdump</a></li>
      <li><a href="clutils-pdbinfo.php">pdbinfo</a></li>
      <li><a href="clutils-randpw.php">randpw</a></li>
      <li><a href="clutils-tryauth.php">tryauth</a></li>
    </ul></li>
  <li>SYSV-style command-line utilities (pwdutils)<br />
    <ul>
      <li><a href="clutils-useradd.php">useradd</a></li>
      <li><a href="clutils-usermod.php">usermod</a></li>
      <li><a href="clutils-userdel.php">userdel</a></li>
      <li><a href="clutils-groupadd.php">groupadd</a></li>
      <li><a href="clutils-groupmod.php">groupmod</a></li>
      <li><a href="clutils-groupdel.php">groupdel</a></li>
    </ul></li>
  <li>CGI programs<br />
    <ul>
      <li><a href="cgiutils-chpasswd.php">chpasswd</a></li>
      <li><a href="cgiutils-vwquota.php">vwquota</a></li>
    </ul></li>
  <li>File formats<br />
    <ul>
      <li><a href="spec-xml.php">XMLDS format</a></li>
      <li><a href="spec-sdf.php">Kolleg SDF format</a></li>
    </ul></li>
  <li>Backends<br />
    <ul>
      <li><a href="driver-mmd.php">MMD</a></li>
      <li><a href="driver-mysql.php">MYSQL</a></li>
      <li><a href="driver-ldap.php">LDAP</a></li>
      <li><a href="driver-shadow.php">Shadow</a></li>
      <li><a href="driver-nss.php">NSS</a></li>
      <li><a href="driver-nss1.php">NSS-1</a></li>
      <li><a href="driver-dummy.php">Dummy</a></li>
    </ul></li>
  <li><a href="extra-pwlstyles.php">List of PWLFMT styles</a></li>
</ul>

</td>
<td valign="top">

<h1>Developer's Reference</h1>

<h2>Outside Vitalnix</h2>

<ul>
  <li><a href="api-libvxcgi.php">libvxcgi</a></li>
  <li><a href="api-libvxcli.php">libvxcli</a></li>
  <li><a href="api-libvxcore.php">libvxcore</a></li>
  <li><a href="api-libvxeds.php">libvxeds</a></li>
  <li><a href="api-libvxmdfmt.php">libvxmdfmt</a></li>
  <li><a href="api-libvxmdsync.php">libvxmdsync</a></li>
  <li><a href="api-libvxpdb.php">libvxpdb</a></li>
  <li><a href="api-libvxplex.php">libvxplex</a></li>
  <li><a href="api-libvxutil.php">libvxutil</a></li>
</ul>

<h2>Inside Vitalnix</h2>

<ul>
  <li><a href="api-mod_backend.php">backend module API</a></li>
  <li><a href="api-mod_edsformat.php">edsformat module API</a></li>
  <li><a href="api-mod_pwlstyle.php">pwlstyle module API</a></li>
  <li><a href="extra-aee.php">Alternate Error Encoding</a></li>
</ul>

</td>
</tr>
</table>

<?php include_once("Base-footer.php"); ?>
