#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <vnet/pg/pg.h>
#include <vnet/ethernet/ethernet.h>
#include <vppinfra/error.h>
#include <pktdump/pktdump.h>

typedef enum
{
  CK_SAMPLE_NEXT_IP4,
  CK_SAMPLE_DROP,
  CK_SAMPLE_NEXT_N,
} ck_sample_next_t;

typedef struct
{
  u32 next_index;
  u32 sw_if_index;
  u8 new_src_mac[6];
  u8 new_dst_mac[6];
} ck_sample_trace_t;

#define foreach_ck_sample_error \
_(SHOWED, "show packets processed")

typedef enum
{
#define _(sym,str) SAMPLE_ERROR_##sym,
  foreach_ck_sample_error
#undef _
    SAMPLE_N_ERROR,
} ck_ssample_error_t;


static char *ck_sample_error_strings[] = {
#define _(sym, str) str,
        foreach_ck_sample_error
#undef _
};

extern vlib_node_registration_t ck_sample_node;

static u8 *
format_ck_sample_trace (u8 * s, va_list * args)
{
        s = format(s, "To Do!\n");
        return s;
}

static uword ck_sample_node_fn(vlib_main_t *vm, vlib_node_runtime_t *node,
        vlib_frame_t * frame)
{
        u32 n_left_from, *from, *to_next;
        ck_sample_next_t     next_index;

        from        = vlib_frame_vector_args(frame); //包指针数组的起始位置
        n_left_from = frame->n_vectors;
        next_index  = node->cached_next_index;

        while(n_left_from > 0){
                u32 n_left_to_next;
                // 用来取你将要传给子节点的包数组信息
                // 检查并确保有足够的空间来传递数据包给下一个节点
                // 这里的 next_index 是你要传递给下一个节点的索引
                // 这里的 to_next 是下一个节点的缓冲区索引数组指针 
                // n_left_to_next 是下一个节点缓冲区中剩余的可用空间 
                // n_left_to_next 是当前帧中剩余的空闲槽位数量,当值为0后，
                //需要vlib_put_next_frame获取一个新的帧来继续传递数据包
                vlib_get_next_frame(vm, node, next_index, to_next, n_left_to_next);

                while(n_left_from > 0 && n_left_to_next > 0){
                        vlib_buffer_t  *b0;
                        u32             bi0, next0 = 0;

                        bi0 = to_next[0] = from[0];
                        from           += 1;
                        to_next        += 1;
                        n_left_to_next -= 1;
                        n_left_from    -= 1;

                        b0 = vlib_get_buffer(vm, bi0);// 获取到包的整个数据
                        
			void *en0 = vlib_buffer_get_current(b0);//获取当前处理的包的数据
                        int i = 0;
                        for (i = 0; i < 34; i++)
                        {
                                printf("%02x ", *(u8*)(en0+i));
                        }
                        printf("\n");
                        // 这一步会验证数据，如果没有符合预期位置，会进行数据复制。
                        //我的业务把这里注释掉了，不调用也可以，只要给to_next写数据即可
                        vlib_validate_buffer_enqueue_x1(vm, node, next_index,
                                to_next, n_left_to_next, bi0, next0);
                }

                // 数据提交到下一节点
                vlib_put_next_frame(vm, node, next_index, n_left_to_next);
        }

        return frame->n_vectors;
}

//  注册一个节点（node） 到 VPP 的处理图
VLIB_REGISTER_NODE (ck_sample_node) = {
        .name		= "ck_sample",
        .function       = ck_sample_node_fn,
        .vector_size    = sizeof(u32),// vector里，每个数据是 4 个字节的 u32 类型
        .format_trace   = format_ck_sample_trace,
        .type           = VLIB_NODE_TYPE_INTERNAL,
        .n_errors       = ARRAY_LEN(ck_sample_error_strings),
        .error_strings  = ck_sample_error_strings,
        .n_next_nodes   = CK_SAMPLE_NEXT_N,
        .next_nodes     = {
                [CK_SAMPLE_NEXT_IP4]    = "ip4-lookup",
                [CK_SAMPLE_DROP]        = "error-drop",
        },
};





