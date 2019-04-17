#include "User.h"
#include "Kernel.h"
#include "Utility.h"

User::User()
{
	/* 进程时间相关 */
	u_utime = 0;
	u_stime= 0;
	u_cutime = 0;
	u_cstime = 0;
}

void User::Setuid()
{
	short uid = this->u_arg[0];
	
	if ( this->u_ruid == uid || this->SUser() )
	{
		this->u_uid = uid;
		this->u_ruid = uid;
	}
	else
	{
		this->u_error = User::MYEPERM;
	}
}

void User::Getuid()
{
	unsigned int uid;

	uid = (this->u_uid << 16);
	uid |= (this->u_ruid & 0xFF);
	this->u_ar0[User::EAX] = uid;
}

void User::Setgid()
{
	short gid = this->u_arg[0];

	if ( this->u_rgid == gid || this->SUser() )
	{
		this->u_gid = gid;
		this->u_rgid = gid;
	}
	else
	{
		this->u_error = User::MYEPERM;
	}
}

void User::Getgid()
{
	unsigned int gid;

	gid = (this->u_gid << 16);
	gid |= (this->u_rgid & 0xFF);
	this->u_ar0[User::EAX] = gid;
}

void User::Pwd()
{
	Utility::StringCopy(this->u_curdir, this->u_dirp);
}

bool User::SUser()
{
	if ( 0 == this->u_uid )
	{
		return true;
	}
	else
	{
		this->u_error = User::MYEPERM;
		return false;
	}
}
