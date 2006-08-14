<?php include_once("Base-header.php"); ?>

<h1>Introduction</h1>

<p class="block">Welcome to Vitalnix 3&nbsp;-- the next generation of the User
Management Suite for Educational Institutions and similar large-size user
bases.</p>

<h1>New features in v3</h1>

<p class="block">Key features of this new major version are the refactoring of
the command-line program into smaller libraries and components for external
applications to use, ease of maintenance, merge of redundant code, support for
NSS-MYSQL and LDAP databases, and the introduction of a graphical user
interface.</p>

<h1>History</h1>

<p class="block">A nameless program (now evolved to the Vitalnix Suite) was
developed in 1997 at the Otto-Hahn-Gymnasium to help adding and deleting
students' accounts in batch <i>based</i> on a file from the secretary's office.
It was basically updating the Unix passwd database with changes from that file.
The first time, it was 1200 users, then about 200 every consecutive year. It
was widely used across the whole town (by four other schools).</p>

<!-- IIRC ... BBS-2 (Berufsbildende Schulen II), MPG (Max-Planck-Gymnasium),
THG (Theodor-Heuss-Gymnasium), BBS (Berd-Brecht-Schule). And of course OHG. -->

<p class="block">It used direct stdio file operations until 2003 when support
for LDAP was requested. Although there is still no LDAP module as of this
writing (the harder part with only one active coder), the architecture has been
thoroughly modularized.</p>

<h1>Authors</h1>

<p class="block">The original Perl scripts were created by Eike Teiwes in
1997-1999.</p>

<p class="block">Jan Engelhardt has picked them up in fall 1999 keeping them up
to date (including designing new from scratch several times) and is the current
maintainer.</p>

<p class="block">I would like to thank the following people for their beta
testing, suggestions and contributions:<br />

Cordula Petzold, Cristoph Thiel, Eberhard Mönkeberg, Hans-Joachim
Bauermeister, Ludger Inhester, Markus Boie,<br />

Extra credit: Solar Designer - Blowfish algorithm (Public Domain).</p>

<?php include_once("Base-footer.php"); ?>
