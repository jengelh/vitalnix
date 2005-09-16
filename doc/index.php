<?php include_once("zheader.php"); ?>

<h1>Description <img src="d_arr.png" /></h1>

<p class="block"><i>Vitalnix</i> is a suite consisting of a library providing  
unified methods to access different user databases (Shadow, LDAP, etc), tools  
for user/group management, and a program for managing users in batch, suitable 
for large systems.</p>

<h1>History <img src="d_arr.png" /></h1>

<p class="block">A nameless program (now called <i>C-Spark</i>) was developed
in 1997 at the Otto-Hahn-Gymnasium (college) to help adding and deleting
students' accounts as they came and went each year. (The first time, it was
1200 users, then about 200 every consecutive year.) It was widely used across
the whole town (4 major colleges).</p>

<p class="block">It used direct file operations until 2003 (<tt>fopen()</tt>,
<tt>fprintf()</tt>), when support for LDAP was requested. So the design of it
ought to change to support multiple back-ends. It turned out to be best to
modularize and separate certain parts, so it was split up into <i>libaccdb</i>
and the UI. Even though the Shadow system is a little different than LDAP, e.g.
the latter requires a hostname in most cases, the trick is to provide a unified
API so C-Spark does not need to know what's behind "add a user with login name
<tt>xyz</tt>, and set the account up.".</p>

<p class="block">A lot of our in-house applications, such as a primitive CGI
authentification used for web-based DiskQuota report and password changing
utility, uses <i>libaccdb</i> to read the user database. This simplifies a
possible change to LDAP in the future, since these applications do not need to
be modified.</p>

<h1>Technical aspects <img src="d_arr.png" /></h1>

<p class="block">Vitalnix and the core parts of it are written in C, though
certain parts of it can be done in C++ too, or even totally different languages
(using C hooks then) if the developer wants to it. It currently runs on Linux
2.4.x, 2.6.x and Microsoft 32-bit Windows on x86 and x86_64 (AMD64).</p>

<p class="block">Current Trove Categorization: (Perl Foundry), Database
Foundry</p>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1">Development Status:</td>
    <td class="t1">5 - Production/Stable</td>
  </tr>
  <tr>
    <td class="t2">Environment:</td>
    <td class="t2">Console (Text Based), No Input/Output (Daemon)</td>
  </tr>
  <tr>
    <td class="t1">Intended Audience:</td>
    <td class="t1">Developers, End Users/Desktop, Information Technology,
      System Administrators</td>
  </tr>
  <tr>
    <td class="t2">License:</td>
    <td class="t2">GNU Lesser Public License (LGPL) v2.1</td>
  </tr>
  <tr>
    <td class="t1">Natural Language:</td>
    <td class="t1">English</td>
  </tr>
  <tr>
    <td class="t2">Operating System:</td>
    <td class="t2">Linux, Windows 95/98/2000</td>
  </tr>
  <tr>
    <td class="t1">Programming Language:</td>
    <td class="t1">C, C++, Perl, Shell, (Assembly)</td>
  </tr>
  <tr>
    <td class="t2">Topic:</td>
    <td class="t2">Database Engines/Servers, Front-Ends, Systems
      Administration</td>
  </tr>
</table>
</div>

<h1>Topic: General <img src="d_arr.png" /></h1>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1"><a href="setup.php">doc/setup.php</a></td>
    <td class="t1">Installing and configuring Vitalnix</td>
  </tr>
</table>
</div>

<h1>Topic: libaccdb - The Unified Account Database <img
src="d_arr.png" /></h1>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t2"><a href="sysprog.php">doc/sysprog.php</a></td>
    <td class="t2">ACCDB system utilities</td>
  </tr>
  <tr>
    <td class="t1"><a href="accdb_api.php">doc/accdb_api.php</a></td>
    <td class="t1">The Unified Account Database API</td>
  </tr>
  <tr>
    <td class="t2"><a href="backend_api.php">doc/backend_api.php</a></td>
    <td class="t2">The back-end module API</td>
  </tr>
</table>
</div>

<h1>Topic: Back-end modules' documentation <img src="d_arr.png" /></h1>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1"><a
      href="backend_shadow.php">doc/backend_shadow.php</a></td>
    <td class="t1">The Shadow back-end</td>
  </tr>
</table>
</div>

<h1>Contents: C-Spark <img src="d_arr.png" /></h1>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t1"><a href="cspark.php">doc/cspark.php</a></td>
    <td class="t1">C-Spark Readme</td>
  </tr>
</table>
</div>

<h1>Other Documents <img src="d_arr.png" /></h1>

<div class="pleft">
<table border="1" class="bordered">
  <tr>
    <td class="t2"><a href="sp_xmlds.php">doc/sp_xmlds.php</a></td>
    <td class="t2">Specifications for SparkUI XML input</td>
  </tr>
  <tr>
    <td class="t1"><a href="faq.php">doc/faq.php</a></td>
    <td class="t1">(FAQ and) Glossary</td>
  </tr>
  <tr>
    <td class="t2"><a href="spec_sdf.php">doc/spec_sdf.php</a></td>
    <td class="t2">SDF specification</td>
  </tr>
  <tr>
    <td class="t1"><a href="dev_todo.txt">doc/dev_todo.txt</a></td>
    <td class="t1">More-or-less comprehensive To-Do list</td>
  </tr>
  <tr>
    <td class="t2"><a href="history.php">doc/history.php</a></td>
    <td class="t2">History time table</td>
  </tr>
  <tr>
    <td class="t1"><a href="LGPL2.txt">doc/LGPL2.txt</a></td>
    <td class="t1">GNU Lesser General Public License (version 2.1)</td>
  </tr>
</table>
</div>

<p class="block">And of course there is the source tree.</p>

<h1>Author and contributors <img src="d_arr.png" /></h1>

<p class="block">The original mass-enregistration Perl scripts were created by
Eike Teiwes &lt;<tt>eteiwes@ohg.goe.net</tt>&gt; in 1997-1999.</p>

<p class="block">Jan Engelhardt &lt;<tt>jengelh@linux01.gwdg.de</tt>&gt;
has picked them up in fall 1999 keeping them up to date (including designing
new from scratch) and is the current maintainer since then.</p>

<p class="block">I would like to thank the following people for their beta-test
and general-support contributions:</p>

<p class="pleft">Cordula Petzold &lt;<tt>cordula.p@gmx.de</tt>&gt;<br />
Eberhard Mönkeberg &lt;<tt>emoenke@gwdg.de</tt>&gt;<br />
Cristoph Thiel &lt;<tt>cthiel1@linux01.gwdg.de</tt>&gt;<br />
Hans-Joachim Bauermeister &lt;<tt>acb@kki.org</tt>&gt;</p>

<p class="pleft">Solar Designer &lt;<tt>solar@openwall.com</tt>&gt;<br />
I included his Blowfish algorithm in Vitalnix, so he is listed!</p>

<p class="block">Other people that have contributed to the very early versions
are:</p>

<p class="pleft">Ludger Inhester &lt;<tt>linhest@linux01.gwdg.de</tt>&gt;<br />
Markus Boie &lt;?&gt;</p>

<?php include_once("zfooter.php"); ?>
