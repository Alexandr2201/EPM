/*
 * "$Id: pkg.c,v 1.18 2001/06/26 16:22:22 mike Exp $"
 *
 *   AT&T package gateway for the ESP Package Manager (EPM).
 *
 *   Copyright 1999-2001 by Easy Software Products.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 * Contents:
 *
 *   make_pkg() - Make an AT&T software distribution package.
 */

/*
 * Include necessary headers...
 */

#include "epm.h"


/*
 * 'make_pkg()' - Make an AT&T software distribution package.
 */

int					/* O - 0 = success, 1 = fail */
make_pkg(const char     *prodname,	/* I - Product short name */
         const char     *directory,	/* I - Directory for distribution files */
         const char     *platname,	/* I - Platform name */
         dist_t         *dist,		/* I - Distribution information */
	 struct utsname *platform)	/* I - Platform information */
{
  int		i;			/* Looping vars */
  FILE		*fp;			/* Control file */
  char		name[1024];		/* Full product name */
  char		filename[1024],		/* Destination filename */
		preinstall[1024],	/* Pre install script */
		postinstall[1024],	/* Post install script */
		preremove[1024],	/* Pre remove script */
		postremove[1024];	/* Post remove script */
  char		current[1024];		/* Current directory */
  file_t	*file;			/* Current distribution file */
  command_t	*c;			/* Current command */
  depend_t	*d;			/* Current dependency */
  tarf_t	*tarfile;		/* Distribution file */
  time_t	curtime;		/* Current time info */
  struct tm	*curdate;		/* Current date info */


  if (Verbosity)
    puts("Creating PKG distribution...");

  if (dist->relnumber)
  {
    if (platname[0])
      snprintf(name, sizeof(name), "%s-%s-%d-%s", prodname, dist->version, dist->relnumber,
              platname);
    else
      snprintf(name, sizeof(name), "%s-%s-%d", prodname, dist->version, dist->relnumber);
  }
  else if (platname[0])
    snprintf(name, sizeof(name), "%s-%s-%s", prodname, dist->version, platname);
  else
    snprintf(name, sizeof(name), "%s-%s", prodname, dist->version);

  getcwd(current, sizeof(current));

 /*
  * Write the pkginfo file for pkgmk...
  */

  if (Verbosity)
    puts("Creating package information file...");

  snprintf(filename, sizeof(filename), "%s/%s.pkginfo", directory, prodname);

  if ((fp = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "epm: Unable to create package information file \"%s\" - %s\n", filename,
            strerror(errno));
    return (1);
  }

  curtime = time(NULL);
  curdate = gmtime(&curtime);

  fprintf(fp, "PKG=%s\n", prodname);
  fprintf(fp, "NAME=%s\n", dist->product);
  fprintf(fp, "VERSION=%s\n", dist->version);
  fprintf(fp, "VENDOR=%s\n", dist->vendor);
  fprintf(fp, "PSTAMP=epm%04d%02d%02d%02d%02d%02d\n",
          curdate->tm_year + 1900, curdate->tm_mon + 1, curdate->tm_mday,
	  curdate->tm_hour, curdate->tm_min, curdate->tm_sec);

  if (dist->num_descriptions > 0)
    fprintf(fp, "DESC=%s\n", dist->descriptions[0]);

  fputs("CATEGORY=application\n", fp);
  fputs("CLASSES=none\n", fp);

  if (strcmp(platform->machine, "intel") == 0)
    fputs("ARCH=i86pc\n", fp);
  else
    fputs("ARCH=sparc\n", fp);

  fclose(fp);

 /*
  * Write the depend file for pkgmk...
  */

  if (Verbosity)
    puts("Creating package dependency file...");

  snprintf(filename, sizeof(filename), "%s/%s.depend", directory, prodname);

  if ((fp = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "epm: Unable to create package dependency file \"%s\" - %s\n", filename,
            strerror(errno));
    return (1);
  }

  for (i = dist->num_depends, d = dist->depends; i > 0; i --, d ++)
    if (d->type == DEPEND_REQUIRES)
      fprintf(fp, "P %s\n", d->product);
    else
      fprintf(fp, "I %s\n", d->product);

  fclose(fp);

 /*
  * Write the preinstall file for pkgmk...
  */

  for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
    if (c->type == COMMAND_PRE_INSTALL)
      break;

  if (i)
  {
   /*
    * Write the preinstall file for pkgmk...
    */

    if (Verbosity)
      puts("Creating preinstall script...");

    snprintf(preinstall, sizeof(preinstall), "%s/%s.preinstall", directory,
             prodname);

    if ((fp = fopen(preinstall, "w")) == NULL)
    {
      fprintf(stderr, "epm: Unable to create script file \"%s\" - %s\n", preinstall,
              strerror(errno));
      return (1);
    }

    fchmod(fileno(fp), 0755);

    fputs("#!/bin/sh\n", fp);
    fputs("# " EPM_VERSION "\n", fp);

    for (; i > 0; i --, c ++)
      if (c->type == COMMAND_PRE_INSTALL)
        fprintf(fp, "%s\n", c->command);

    fclose(fp);
  }
  else
    preinstall[0] = '\0';

 /*
  * Write the postinstall file for pkgmk...
  */

  for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
    if (c->type == COMMAND_POST_INSTALL)
      break;

  if (!i)
    for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
      if (tolower(file->type) == 'i')
        break;

  if (i)
  {
   /*
    * Write the postinstall file for pkgmk...
    */

    if (Verbosity)
      puts("Creating postinstall script...");

    snprintf(postinstall, sizeof(postinstall), "%s/%s.postinstall", directory,
             prodname);

    if ((fp = fopen(postinstall, "w")) == NULL)
    {
      fprintf(stderr, "epm: Unable to create script file \"%s\" - %s\n", postinstall,
              strerror(errno));
      return (1);
    }

    fchmod(fileno(fp), 0755);

    fputs("#!/bin/sh\n", fp);
    fputs("# " EPM_VERSION "\n", fp);

    for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
      if (c->type == COMMAND_POST_INSTALL)
        fprintf(fp, "%s\n", c->command);

    for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
      if (tolower(file->type) == 'i')
	fprintf(fp, "/etc/init.d/%s start\n", file->dst);

    fclose(fp);
  }
  else
    postinstall[0] = '\0';

 /*
  * Write the preremove script...
  */

  for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
    if (c->type == COMMAND_PRE_REMOVE)
      break;

  if (!i)
    for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
      if (tolower(file->type) == 'i')
        break;

  if (i)
  {
   /*
    * Write the preremove file for pkgmk...
    */

    if (Verbosity)
      puts("Creating preremove script...");

    snprintf(preremove, sizeof(preremove), "%s/%s.preremove", directory, prodname);

    if ((fp = fopen(preremove, "w")) == NULL)
    {
      fprintf(stderr, "epm: Unable to create script file \"%s\" - %s\n", preremove,
              strerror(errno));
      return (1);
    }

    fchmod(fileno(fp), 0755);

    fputs("#!/bin/sh\n", fp);
    fputs("# " EPM_VERSION "\n", fp);

    for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
      if (tolower(file->type) == 'i')
	fprintf(fp, "/etc/init.d/%s stop\n", file->dst);

    for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
      if (c->type == COMMAND_PRE_REMOVE)
        fprintf(fp, "%s\n", c->command);

    fclose(fp);
  }
  else
    preremove[0] = '\0';

 /*
  * Write the postremove file for pkgmk...
  */

  for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
    if (c->type == COMMAND_POST_REMOVE)
      break;

  if (i)
  {
   /*
    * Write the postremove file for pkgmk...
    */

    if (Verbosity)
      puts("Creating postremove script...");

    snprintf(postremove, sizeof(postremove), "%s/%s.postremove", directory,
             prodname);

    if ((fp = fopen(postremove, "w")) == NULL)
    {
      fprintf(stderr, "epm: Unable to create script file \"%s\" - %s\n", postremove,
              strerror(errno));
      return (1);
    }

    fchmod(fileno(fp), 0755);

    fputs("#!/bin/sh\n", fp);
    fputs("# " EPM_VERSION "\n", fp);

    for (; i > 0; i --, c ++)
      if (c->type == COMMAND_POST_REMOVE)
        fprintf(fp, "%s\n", c->command);

    fclose(fp);
  }
  else
    postremove[0] = '\0';

 /*
  * Add symlinks for init scripts...
  */

  for (i = 0; i < dist->num_files; i ++)
    if (tolower(dist->files[i].type) == 'i')
    {
      file = add_file(dist);
      file->type = 'l';
      file->mode = 0;
      strcpy(file->user, "root");
      strcpy(file->group, "sys");
      snprintf(file->src, sizeof(file->src), "../init.d/%s", dist->files[i].dst);
      snprintf(file->dst, sizeof(file->dst), "/etc/rc0.d/K00%s", dist->files[i].dst);

      file = add_file(dist);
      file->type = 'l';
      file->mode = 0;
      strcpy(file->user, "root");
      strcpy(file->group, "sys");
      snprintf(file->src, sizeof(file->src), "../init.d/%s", dist->files[i].dst);
      snprintf(file->dst, sizeof(file->dst), "/etc/rc3.d/S99%s", dist->files[i].dst);

      file = dist->files + i;

      snprintf(filename, sizeof(filename), "/etc/init.d/%s", file->dst);
      strcpy(file->dst, filename);
    }

 /*
  * Write the prototype file for pkgmk...
  */

  if (Verbosity)
    puts("Creating prototype file...");

  snprintf(filename, sizeof(filename), "%s/%s.prototype", directory, prodname);

  if ((fp = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "epm: Unable to create prototype file \"%s\" - %s\n",
            filename, strerror(errno));
    return (1);
  }

  if (dist->license[0] == '/')
    fprintf(fp, "i copyright=%s\n", dist->license);
  else
    fprintf(fp, "i copyright=%s/%s\n", current, dist->license);

  fprintf(fp, "i depend=%s/%s/%s.depend\n", current, directory, prodname);
  fprintf(fp, "i pkginfo=%s/%s/%s.pkginfo\n", current, directory, prodname);

  for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
    if (tolower(file->type) == 'i')
      break;

  if (preinstall[0])
    fprintf(fp, "i preinstall=%s/%s\n", current, preinstall);
  if (postinstall[0])
    fprintf(fp, "i postinstall=%s/%s\n", current, postinstall);
  if (preremove[0])
    fprintf(fp, "i preremove=%s/%s\n", current, preremove);
  if (postremove[0])
    fprintf(fp, "i postremove=%s/%s\n", current, postremove);

  for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
    switch (tolower(file->type))
    {
      case 'c' :
          if (file->src[0] == '/')
            fprintf(fp, "e none %s=%s %04o %s %s\n", file->dst,
	            file->src, file->mode, file->user, file->group);
          else
            fprintf(fp, "e none %s=%s/%s %04o %s %s\n", file->dst,
	            current, file->src, file->mode, file->user, file->group);
          break;
      case 'd' :
	  fprintf(fp, "d none %s %04o %s %s\n", file->dst, file->mode,
		  file->user, file->group);
          break;
      case 'f' :
      case 'i' :
          if (file->src[0] == '/')
            fprintf(fp, "f none %s=%s %04o %s %s\n", file->dst,
	            file->src, file->mode, file->user, file->group);
          else
            fprintf(fp, "f none %s=%s/%s %04o %s %s\n", file->dst,
	            current, file->src, file->mode, file->user, file->group);
          break;
      case 'l' :
          fprintf(fp, "s none %s=%s\n", file->dst, file->src);
          break;
    }

  fclose(fp);

 /*
  * Build the distribution from the prototype file...
  */

  if (Verbosity)
    puts("Building PKG binary distribution...");

  if (run_command(NULL, "pkgmk -o -f %s/%s.prototype -d %s/%s",
                  directory, prodname, current, directory))
    return (1);

 /*
  * Tar and compress the distribution...
  */

  if (Verbosity)
    puts("Creating tar.gz file for distribution...");

  snprintf(filename, sizeof(filename), "%s/%s.tar.gz", directory, name);

  if ((tarfile = tar_open(filename, 1)) == NULL)
    return (1);

  snprintf(filename, sizeof(filename), "%s/%s", directory, prodname);

  if (tar_directory(tarfile, filename, prodname))
  {
    tar_close(tarfile);
    return (1);
  }

  tar_close(tarfile);

 /*
  * Make a package stream file...
  */

  if (Verbosity)
    puts("Copying into package stream file...");

  if (run_command(directory, "pkgtrans -s . %s.pkg %s", name, prodname))
    return (1);

 /*
  * Remove temporary files...
  */

  if (!KeepFiles)
  {
    if (Verbosity)
      puts("Removing temporary distribution files...");

    snprintf(filename, sizeof(filename), "%s/%s.pkginfo", directory, prodname);
    unlink(filename);
    snprintf(filename, sizeof(filename), "%s/%s.depend", directory, prodname);
    unlink(filename);
    snprintf(filename, sizeof(filename), "%s/%s.prototype", directory, prodname);
    unlink(filename);
    if (preinstall[0])
      unlink(preinstall);
    if (postinstall[0])
      unlink(postinstall);
    if (preremove[0])
      unlink(preremove);
    if (postremove[0])
      unlink(postremove);
  }

  return (0);
}


/*
 * End of "$Id: pkg.c,v 1.18 2001/06/26 16:22:22 mike Exp $".
 */