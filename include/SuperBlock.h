/*
 * 文件系统存储资源管理块(Super Block)的定义。
 */
class SuperBlock
{
	/* Functions */
public:
	/* Constructors */
	SuperBlock(){};
	/* Destructors */
	~SuperBlock(){};
	
	/* Members */
public:
	int		s_isize;		/* Inode区占用的盘块数 */
	int		s_dsize;		/* data区占用盘块数 */
	int		s_fsize;		/* 盘块总数 */
	
	int		s_dstart;		/* 数据区起始盘块 */
	int		s_ndfree;		/* 直接管理的空闲数据块块数量 */
	int		s_ninode;		/* 直接管理的空闲外存Inode数量 */
	
	int		s_flock;		/* 封锁空闲盘块索引表标志 */
	int		s_ilock;		/* 封锁空闲Inode表标志 */
	
	int		s_fmod;			/* 内存中super block副本被修改标志，意味着需要更新外存对应的Super Block */
	int		s_ronly;		/* 本文件系统只能读出 */
	int		s_time;			/* 最近一次更新时间 */
	
	int		s_reserved[21];	/* 填充使SuperBlock块大小等于128字节，占据1个扇区 */
};