<?php include_once("Base-header.php"); ?>

<table>
<tr>
<td valign="top">

<h1>Administrator's Reference</h1>

<ul>
  <li><a href="gen-intro.php">Introduction</a></li>
  <li><a href="gen-tech.php">Technical aspects</a></li>
</ul>

<h2>Print accounting</h2>

<ul>
  <li><a href="lpacct.php">Print accounting</a></li>
</ul>

<h2>Manpages</h2>

<ul>
  <li>Shell utilities (clutils)<br />
    <ul>
      <li><a href="vxdbdump.8.php">vxdbdump</a></li>
      <li><a href="vxdbinfo.8.php">vxdbinfo</a></li>
      <li><a href="vxfixuuid.8.php">vxfixuuid</a></li>
      <li><a href="vxpwlfmt.8.php">vxpwlfmt</a></li>
      <li><a href="vxsingle.8.php">vxsingle</a></li>
      <li><a href="vxusersync.8.php">vxusersync</a></li>
      <li><a href="vxrandpw.1.php">vxrandpw</a></li>
      <li><a href="vxtryauth.8.php">vxtryauth</a></li>
    </ul></li>
  <li>SYSV-style command-line utilities (pwdutils)<br />
    <ul>
      <li><a href="vxuseradd.8.php">vxuseradd</a></li>
      <li><a href="vxusermod.8.php">vxusermod</a></li>
      <li><a href="vxuserdel.8.php">vxuserdel</a></li>
      <li><a href="vxgroupadd.8.php">vxgroupadd</a></li>
      <li><a href="vxgroupmod.8.php">vxgroupmod</a></li>
      <li><a href="vxgroupdel.8.php">vxgroupdel</a></li>
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
      <li><a href="vxdrv_mmd.7.php">MMD</a></li>
      <li><a href="vxdrv_mysql.7.php">MYSQL</a></li>
      <li><a href="vxdrv_ldap.7.php">LDAP</a></li>
      <li><a href="vxdrv_shadow.7.php">Shadow</a></li>
      <li><a href="vxdrv_nss.7.php">NSS and NSS-1</a></li>
      <li><a href="vxdrv_dummy.7.php">Dummy</a></li>
    </ul></li>
  <li><a href="extra-pwlstyles.php">List of PWLFMT styles</a></li>
</ul>

</td>
<td valign="top">

<h1>Developer's Reference</h1>

<h2>External API</h2>

<ul>
  <li><a href="api-libvxcgi.php">libvxcgi</a></li>
  <li><a href="api-libvxcli.php">libvxcli</a></li>
  <li><a href="api-libvxcore.php">libvxcore</a></li>
  <li><a href="api-libvxdb.php">libvxdb</a></li>
  <li><a href="api-libvxeds.php">libvxeds</a></li>
  <li><a href="api-libvxmdfmt.php">libvxmdfmt</a></li>
  <li><a href="api-libvxmdsync.php">libvxmdsync</a></li>
  <li><a href="api-libvxutil.php">libvxutil</a></li>
</ul>

<h2>Internal API</h2>

<ul>
  <li><a href="api-mod_backend.php">backend module API</a></li>
  <li><a href="extra-aee.php">Alternate Error Encoding</a></li>
</ul>

</td>
</tr>
</table>

<?php include_once("Base-footer.php"); ?>
