/**
 ****************************************************************************************
 *
 * @file gapc_int.h
 *
 * @brief Generic Access Profile Controller Internal Header.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 ****************************************************************************************
 */
#ifndef _GAPC_INT_H_
#define _GAPC_INT_H_

/**
 ****************************************************************************************
 * @addtogroup GAPC_INT Generic Access Profile Controller Internals
 * @ingroup GAPC
 * @brief Handles ALL Internal GAPC API
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#if (BLE_GAPC)
#include "gapc.h"
#include "gapc_msg.h"
#include "../../inc/gap_hl_api.h"
#include "common_bt.h"
#include "common_buf.h"
#include "common_time.h"
#include "common_djob.h"
#include "kernel_task.h"

/*
 * MACROS
 ****************************************************************************************
 */

/// GAP Attribute database handles
/// Generic Access Profile Service
enum
{
    // GATT Service index
    GATT_IDX_PRIM_SVC,

    GATT_IDX_CHAR_SVC_CHANGED,
    GATT_IDX_SVC_CHANGED,
    GATT_IDX_SVC_CHANGED_CFG,

    GATT_IDX_CHAR_CLI_SUP_FEAT,
    GATT_IDX_CLI_SUP_FEAT,

    GATT_IDX_CHAR_DB_HASH,
    GATT_IDX_DB_HASH,

    GATT_IDX_CHAR_SRV_SUP_FEAT,
    GATT_IDX_SRV_SUP_FEAT,

    GATT_IDX_NUMBER,

    // GAP Service index
    GAP_IDX_PRIM_SVC = GATT_IDX_NUMBER,

    GAP_IDX_CHAR_DEVNAME,
    GAP_IDX_DEVNAME,

    GAP_IDX_CHAR_ICON,
    GAP_IDX_ICON,

    GAP_IDX_CHAR_RSLV_PRIV_ADDR_ONLY,
    GAP_IDX_RSLV_PRIV_ADDR_ONLY,

    GAP_IDX_CHAR_SLAVE_PREF_PARAM,
    GAP_IDX_SLAVE_PREF_PARAM,

    GAP_IDX_CHAR_CNT_ADDR_RESOL,
    GAP_IDX_CNT_ADDR_RESOL,

    GAP_IDX_NUMBER,

    /// Maximum number of GATT attributes
    GATT_NB_ATT = GATT_IDX_NUMBER,
    /// Maximum number of GAP attributes
    GAP_NB_ATT = GAP_IDX_NUMBER - GATT_IDX_NUMBER,
};

/*
 * DEFINES
 ****************************************************************************************
 */

/// number of GAP Controller Process
#define GAPC_IDX_MAX                                 BLE_CONNECTION_MAX


/**
 * Repeated Attempts Timer Configuration
 */
/// Repeated Attempts Timer default value
#define GAPC_SMP_REP_ATTEMPTS_TIMER_DEF_VAL_MS         (2000)      //2s
/// Repeated Attempts Timer max value )
#define GAPC_SMP_REP_ATTEMPTS_TIMER_MAX_VAL_MS         (30000)     //30s
/// Repeated Attempts Timer multiplier
#define GAPC_SMP_REP_ATTEMPTS_TIMER_MULT               (2)

/// GAP Supported client feature mask
#define GAPC_CLI_FEAT_SUPPORTED_MASK                   (0x07)
/// GAP Supported server feature mask
#define GAPC_SRV_FEAT_SUPPORTED_MASK                   (0x01)

/// size of bit field array according to number of connection
#define GAPC_CON_ARRAY_BF_SIZE  ((BLE_CONNECTION_MAX + 7) / 8)




/// Operation type
enum gapc_op_type
{
    /// Operation used to manage Link (update params, get peer info)
    GAPC_OP_LINK_INFO    = 0x00,

    /// Operation used to manage SMP
    GAPC_OP_SMP          = 0x01,

    /// Operation used to manage connection update
    GAPC_OP_LINK_UPD     = 0x02,

    /// Max number of operations
    GAPC_OP_MAX
};

/// states of GAP Controller task
enum gapc_state_id
{
    /// Connection ready state
    GAPC_READY,

    /// Link Operation on-going
    GAPC_LINK_INFO_BUSY     = (1 << GAPC_OP_LINK_INFO),
    /// SMP Operation  on-going
    GAPC_SMP_BUSY           = (1 << GAPC_OP_SMP),
    /// Update Operation  on-going
    GAPC_LINK_UPD_BUSY      = (1 << GAPC_OP_LINK_UPD),
    /// SMP start encryption on-going
    GAPC_ENCRYPT_BUSY       = (1 << GAPC_OP_MAX),

    /// Disconnection  on-going
    GAPC_DISC_BUSY          = 0x1F,
    /// Free state
    GAPC_FREE               = 0X3F,

    /// Number of defined states.
    GAPC_STATE_MAX
};

/// fields definitions.
/// Configuration fields:
///  15-9   8     7    6    5    4    3    2    1    0
/// +----+-----+-----+----+----+----+----+----+----+----+
/// |RFU |L_RPA|BDATA|SCEN|ROLE|LTK |ENC |BOND| SEC_LVL |
/// +----+-----+-----+----+----+----+----+----+----+----+
enum gapc_fields
{
    /// Link Security Level
    GAPC_SEC_LVL_LSB           = 0,
    GAPC_SEC_LVL_MASK          = 0x0003,
    /// Link Bonded or not
    GAPC_BONDED_POS            = 2,
    GAPC_BONDED_BIT            = 0x0004,
    /// Encrypted connection or not
    GAPC_ENCRYPTED_POS         = 3,
    GAPC_ENCRYPTED_BIT         = 0x0008,
    /// Ltk present and exchanged during pairing
    GAPC_LTK_PRESENT_POS       = 4,
    GAPC_LTK_PRESENT_BIT       = 0x0010,
    /// Local connection role
    GAPC_ROLE_POS              = 5,
    GAPC_ROLE_BIT              = 0x0020,
    /// Used to know if secure connection is enabled
    GAPC_SC_EN_POS             = 6,
    GAPC_SC_EN_BIT             = 0x0040,
    /// Used to know if bond data has been updated at client or server level
    GAPC_BOND_DATA_UPDATED_POS = 7,
    GAPC_BOND_DATA_UPDATED_BIT = 0x0080,
    /// Local Address is an RPA generated by controller
    GAPC_LOCAL_RPA_POS         = 8,
    GAPC_LOCAL_RPA_BIT         = 0x0100,
};

/// GAP Client and Server configuration bit field
///
///     7    6  5     4        3       2     1      0
/// +-------+--+--+------+----------+-----+------+-----+
/// | D_B_I | RFU | EATT | CLI_FEAT | MTU | RPAO | PCP |
/// +-------+--+--+------+----------+-----+------+-----+
///
/// - Bit [0]  : Preferred Connection parameters present in GAP DB
/// - Bit [1]  : Presence of Resolvable private address only in GAP DB
/// - Bit [2]  : Automatically start MTU exchange at connection establishment
/// - Bit [3]  : Enable automatic robust cache enable at connection establishment
/// - Bit [4]  : Automatically start establishment of Enhanced ATT bearers
/// - Bit [7]  : Trigger bond information to application even if devices are not bonded
enum gapc_cfg_bf
{
    /// Preferred Connection parameters present in GAP DB
    GAPC_SVC_PREF_CON_PAR_PRES_BIT          = 0x01,
    GAPC_SVC_PREF_CON_PAR_PRES_POS          = 0,
    /// Presence of Resolvable private address only in GAP DB
    GAPC_SVC_RSLV_PRIV_ADDR_ONLY_PRES_BIT   = 0x02,
    GAPC_SVC_RSLV_PRIV_ADDR_ONLY_PRES_POS   = 1,
    /// Automatically start MTU exchange at connection establishment
    GAPC_CLI_AUTO_MTU_EXCH_EN_BIT           = 0x04,
    GAPC_CLI_AUTO_MTU_EXCH_EN_POS           = 2,
    /// Enable automatic client features enabling at connection establishment
    GAPC_CLI_AUTO_FEAT_EN_BIT               = 0x08,
    GAPC_CLI_AUTO_FEAT_EN_POS               = 3,
    /// Automatically start establishment of Enhanced ATT bearers
    GAPC_CLI_AUTO_EATT_BIT                  = 0x10,
    GAPC_CLI_AUTO_EATT_POS                  = 4,
    /// Trigger bond information to application even if devices are not bonded
    GAPC_DBG_BOND_INFO_TRIGGER_BIT          = 0x80,
    GAPC_DBG_BOND_INFO_TRIGGER_POS          = 7,
};



/*
 * TYPE DECLARATIONS
 ****************************************************************************************
 */

/// Pairing Information
typedef struct gapc_smp_pair_info gapc_smp_pair_info_t;

/// GAPC_SMP environment structure
typedef struct gapc_smp
{
    /// transaction time
    gapc_sdt_t            trans_timer;
    ///TRamsaction failed repeated attempt timer
    gapc_sdt_t            rep_attempt_timer;

    /// Pairing Information - This structure is allocated at the beginning of a pairing
    /// or procedure. It is freed when a disconnection occurs or at the end of
    /// the pairing procedure. If not enough memory can be found, the procedure will fail
    ///  with an "Unspecified Reason" error
    gapc_smp_pair_info_t* pair;

    /// Repeated Attempt Timer value
    uint16_t              rep_att_timer_val;

    /// Encryption key size
    uint8_t               key_size;

    /// Contains the current state of the two timers needed in the GAPC_SMP task
    ///      Bit 0 - Is Timeout Timer running
    ///      Bit 1 - Is Repeated Attempt Timer running
    ///      Bit 2 - Has task reached a SMP Timeout
    uint8_t               timer_state;

    /// State of the current procedure
    uint8_t               state;

    /// SMP channel local identifier
    uint8_t               chan_lid;
} gapc_smp_t;

typedef struct gapc_bond_
{
    /// CSRK values (Local and remote)
    gap_sec_key_t         csrk[GAPC_SMP_INFO_MAX];

    /// signature counter values (Local and remote)
    uint32_t              sign_counter[GAPC_SMP_INFO_MAX];

    #if (BLE_GATT_CLI)
    /// GATT Service Start handle
    uint16_t              gatt_start_hdl;
    /// Number of attributes present in GATT service
    uint8_t               gatt_nb_att;
    /// Service Change value handle offset
    uint8_t               svc_chg_offset;
    /// Server supported feature bit field (@see enum gapc_srv_feat)
    uint8_t               srv_feat;
    #endif // (BLE_GATT_CLI)

    /// Client supported feature bit field (@see enum gapc_cli_feat)
    uint8_t               cli_feat;
} gapc_bond_t;

/// GAP controller connection variable structure.
typedef struct gapc_con
{
    /// Request operation Kernel message
    void*               operation[GAPC_OP_MAX];
    /// Source task id of requested disconnection
    kernel_task_id_t        disc_requester;
    /// Destination task ID for asynchronous events not linked to an operation
    kernel_task_id_t        dest_task_id;
    /* Connection parameters to keep */
    /// Bond data for the connections
    gapc_bond_t         bond;
    /// Security Management Protocol environment variables
    gapc_smp_t          smp;
    /// connection handle
    uint16_t            conhdl;
    /// Configuration fields (@see enum gapc_fields) // TODO [FBE] field to put in bond data to provide authentication level
    uint16_t            fields;
    /// BD Address used for the link that should be kept
    gap_bdaddr_t        src[GAPC_SMP_INFO_MAX];
    /// LE features 8-byte array supported by peer device
    uint8_t             peer_features[GAP_LE_FEATS_LEN];
    /// Channel Selection Algorithm
    uint8_t             chan_sel_algo;
} gapc_con_t;


/// GAPM Service environment
typedef struct gapc_svc
{
    /// Pointer to the buffer used for service changed indication
    common_buf_t* p_svc_chg_ind_buf;
    /// Token identifier of Service change indication
    uint16_t  svc_chg_ind_token;
    /// GAP service start handle
    uint16_t  gap_start_hdl;
    /// GATT service start handle
    uint16_t  gatt_start_hdl;
    /// Bit field that contains service change event registration bit field (1 bit per connection)
    uint8_t   svc_chg_ccc_bf[GAPC_CON_ARRAY_BF_SIZE];
    /// Bit field that contains client aware status (1 bit per connection)
    uint8_t   cli_chg_aware_bf[GAPC_CON_ARRAY_BF_SIZE];
    /// Bit field that contains client attribute request authorization(1 bit per connection)
    uint8_t   cli_att_req_allowed_bf[GAPC_CON_ARRAY_BF_SIZE];
    /// GATT user local identifier for service
    uint8_t   user_lid;
} gapc_svc_t;


/// GAP controller environment variable structure.
typedef struct gapc_env_
{
    /// Pointer to connection environment
    gapc_con_t*     p_con[BLE_CONNECTION_MAX];
    /// GAP / GATT service environment
    gapc_svc_t      svc;
    /// Main SDT delayed job.
    common_djob_t       sdt_job;
    /// Defer Job queue
    common_list_t       sdt_job_queue;
    /// Main SDT timer
    common_time_timer_t sdt_timer;
    /// Timer queue
    common_list_t       sdt_timer_queue;
    /// Last timer duration (used to speed up timer insertion)
    uint16_t        sdt_max_push_duration;
    /// Timer and defer bit field
    uint8_t         sdt_state_bf;
    #if (BLE_GATT_CLI)
    /// GAP / GATT client environment
    uint8_t         cli_user_lid;
    #endif // (BLE_GATT_CLI)
    /// Client and server configuration - (@see enum gapc_cfg_bf)
    uint8_t         cfg_flags;
} gapc_env_t;

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern gapc_env_t gapc_env;


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Send Disconnection indication to specific task
 *
 * @param[in] conidx  Connection index
 * @param[in] reason  Disconnection reason
 * @param[in] conhdl  Connection handle
 * @param[in] dest_id Message destination ID
 *
 ****************************************************************************************
 */
void gapc_send_disconect_ind(uint8_t conidx,  uint8_t reason, uint8_t conhdl,
                             kernel_task_id_t dest_id);



/**
 * @brief Send a complete event of ongoing executed operation to requester.
 * It also clean-up variable used for ongoing operation.
 *
 * @param[in] conidx Connection index
 * @param[in] op_type       Operation type.
 * @param[in] status Status of completed operation
 */
void gapc_send_complete_evt(uint8_t conidx, uint8_t op_type, uint8_t status);

/**
 ****************************************************************************************
 * @brief Send operation completed message with status error code not related to a
 * running operation.
 *
 * @param[in] conidx    Connection index
 * @param[in] operation Operation code
 * @param[in] requester requester of operation
 * @param[in] status    Error status code
 ****************************************************************************************
 */
void gapc_send_error_evt(uint8_t conidx, uint8_t operation, const kernel_task_id_t requester, uint8_t status);


/**
 ****************************************************************************************
 * @brief Get operation on going
 *
 * @param[in] conidx        Connection Index
 * @param[in] op_type       Operation type.
 *
 * @return operation code on going
 ****************************************************************************************
 */
uint8_t gapc_get_operation(uint8_t conidx, uint8_t op_type);

/**
 ****************************************************************************************
 * @brief Get operation pointer
 *
 * @param[in] conidx        Connection Index
 * @param[in] op_type       Operation type.
 *
 * @return operation pointer on going
 ****************************************************************************************
 */
void* gapc_get_operation_ptr(uint8_t conidx, uint8_t op_type);


/**
 ****************************************************************************************
 * @brief Set operation pointer
 *
 * @param[in] conidx        Connection Index
 * @param[in] op_type       Operation type.
 * @param[in] op            Operation pointer.
 *
 ****************************************************************************************
 */
void gapc_set_operation_ptr(uint8_t conidx, uint8_t op_type, void* op);

/**
 ****************************************************************************************
 * @brief Operation execution not finish, request kernel to reschedule it in order to
 * continue its execution
 *
 * @param[in] conidx        Connection Index
 * @param[in] op_type       Operation type.
 *
 * @return if operation has been rescheduled (not done if operation pointer is null)
 ****************************************************************************************
 */
bool gapc_reschedule_operation(uint8_t conidx, uint8_t op_type);

/**
 * Clean-up operation parameters
 *
 * @param[in] conidx connection index
 * @param[in] op_type       Operation type.
 */
void gapc_operation_cleanup(uint8_t conidx, uint8_t op_type);

/**
 ****************************************************************************************
 * @brief Get requester of on going operation
 *
 * @param[in] conidx        Connection Index
 * @param[in] op_type       Operation type.
 *
 * @return ID of task that requested to execute the operation
 ****************************************************************************************
 */
kernel_task_id_t gapc_get_requester(uint8_t conidx, uint8_t op_type);

/**
 ****************************************************************************************
 * @brief Update link security level
 *
 * @param[in] conidx            Connection index
 * @param[in] link_encrypted    Use to know if link is encrypted
 * @param[in] pairing_lvl       Link pairing level
 * @param[in] ltk_present       Link paired and an LTK has been exchanged
 *
 ****************************************************************************************
 */
void gapc_sec_lvl_set(uint8_t conidx, bool link_encrypted, uint8_t pairing_lvl, bool ltk_present);

/**
 ****************************************************************************************
 * @brief Retrieve link pairing level
 *
 * @param[in] conidx Connection index
 * @return Link pairing level
 ****************************************************************************************
 */
uint8_t gapc_pairing_lvl_get(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Update task state
 *
 * @param[in] conidx Connection index
 * @param[in] state to update
 * @param[in] set state to busy (true) or idle (false)
 *
 ****************************************************************************************
 */
void gapc_update_state(uint8_t conidx, kernel_state_t state, bool busy);

/**
 ****************************************************************************************
 * @brief Get channel selection algorithm used for a given connection identified
 * by its connection index
 *
 * @param[in] conidx    Connection index
 *
 * @return Channel selection algorithm used (0 if algo #1, 1 if algo #2)
 ****************************************************************************************
 */
uint8_t gapc_get_chan_sel_algo(uint8_t conidx);


/**
 ****************************************************************************************
 * @brief Clean-up SMP operation
 *
 * @param[in] conidx        Connection Index
 ****************************************************************************************
 */
void gapc_smp_op_cleanup(uint8_t conidx);


/**
 ****************************************************************************************
 * @brief Handle connection creation
 *
 * @param[in] conidx        Connection Index
 ****************************************************************************************
 */
void gapc_smp_create(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Check if current operation can be processed or not.
 * if it can be proceed, initialize an operation request.
 * If a command complete event with error code can be triggered.
 *
 * Function returns how the message should be handled by message handler.
 *
 * @param[in] conidx        Connection Index
 * @param[in] op_type       Operation type.
 * @param[in] op_msg        Requested operation message (note op_msg cannot be null)
 * @param[in] supp_ops      Supported operations array.
 *                          Latest array value shall be GAPC_NO_OP.
 *
 * @return operation can be executed if message status equals KERNEL_MSG_NO_FREE,
 * else nothing to do, just exit from the handler.
 ****************************************************************************************
 */
int gapc_process_op(uint8_t conidx, uint8_t op_type, void* op_msg, enum gapc_operation* supp_ops);

/**
 ****************************************************************************************
 * @brief Inform Application about read peer information
 *
 * @param[in] conidx        Connection Index
 * @param[in] operation     On-going operation (@see enum gapc_operation)
 * @param[in] hdl           Attribute handle
 * @param[in] length        Value length
 * @param[in] p_val         Pointer to attribute value
 ****************************************************************************************
 */
void gapc_info_send(uint8_t conidx, uint8_t operation, uint16_t hdl, uint16_t length, const uint8_t* p_val);

/**
 ****************************************************************************************
 * @brief Inform Application about update of bond information
 *
 * @param[in] conidx        Connection Index
 ****************************************************************************************
 */
void gapc_bond_info_send(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief inform GAP/GATT service that a connection has been established
 *
 * @param[in] conidx             Connection index
 * @param[in] bond_data_present  True if bond data present, False else
 * @param[in] cli_info           Client bond data information (@see enum gapc_cli_info)
 * @param[in] cli_feat           Client supported features    (@see enum gapc_cli_feat)
 ****************************************************************************************
 */
void gapc_svc_con_create(uint8_t conidx, bool bond_data_present, uint8_t cli_info, uint8_t cli_feat);

#if (BLE_GATT_CLI)

/**
 ****************************************************************************************
 * @brief Inform GAP/GATT client that a connection has been established
 *
 * @param[in] conidx             Connection index
 * @param[in] bond_data_present  True if bond data present, False else
 * @param[in] gatt_start_hdl     GATT service start handle
 * @param[in] gatt_end_hdl       GATT Service end handle
 * @param[in] svc_chg_hdl        GATT Service changed value handle
 * @param[in] srv_feat           Server supported features (@see enum gapc_srv_feat)
 ****************************************************************************************
 */
void gapc_cli_con_create(uint8_t conidx, bool bond_data_present, uint16_t gatt_start_hdl, uint16_t gatt_end_hdl,
                        uint16_t svc_chg_hdl, uint8_t srv_feat);

/**
 ****************************************************************************************
 * @brief Ask GAP/GATT client to retrieve some peer device information
 *
 * @param[in] conidx    Connection index
 * @param[in] operation GAP connection operation (@see enum gapc_operation)
 *
 * @return Execution status (@see enum hl_err)
 ****************************************************************************************
 */
uint16_t gapc_cli_info_get(uint8_t conidx, uint8_t operation);

/**
 ****************************************************************************************
 * @brief Function called when link becomes encrypted.
 *
 * @param[in] conidx    Connection index
 ****************************************************************************************
 */
void gapc_cli_link_encrypted(uint8_t conidx);
#endif // (BLE_GATT_CLI)

/**
 ****************************************************************************************
 * @brief Retrieve Bond Data information
 *
 * @param[in]  conidx     Connection index
 * @param[out] p_cli_info Pointer to client information (@see enum gapc_cli_info)
 ****************************************************************************************
 */
void gapc_svc_bond_data_get(uint8_t conidx, uint8_t* p_cli_info);

#if (BLE_PWR_CTRL)
/**
 ****************************************************************************************
 * @brief Get local connection tx power level
 *
 * @param[in]  conidx     Connection index
 * @param[in]  operation  Information operation code
 ****************************************************************************************
 */
void gapc_pwr_ctrl_loc_tx_lvl_get(uint8_t conidx, uint8_t operation);

/**
 ****************************************************************************************
 * @brief Get remove connection tx power level
 *
 * @param[in]  conidx     Connection index
 * @param[in]  operation  Information operation code
 ****************************************************************************************
 */
void gapc_pwr_ctrl_peer_tx_lvl_get(uint8_t conidx, uint8_t operation);
#endif // (BLE_PWR_CTRL)


/**
 ****************************************************************************************
 * @brief Get destination task id for asynchronous event, meaning events that are not
 * linked to an operation.
 * Note the provided connection index shall be valid (gapc_env[conidx] is not NULL)
 *
 * @param[in] conidx        Connection Index
 *
 * @return ID of the destination task.
 ****************************************************************************************
 */
kernel_task_id_t gapc_get_dest_task(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Retrieve connection address information on current link.
 *
 * @param[in] conidx Connection index
 * @param[in] src    Connection information source
 *
 * @return Return found connection address
 ****************************************************************************************
 */
struct gap_bdaddr* gapc_get_bdaddr(uint8_t conidx, uint8_t src);


/**
 ****************************************************************************************
 * @brief InitializeGAPC Simple Timer and Defer module
 *
 * @param[in] init_type  Type of initialization (@see enum rwip_init_type)
 *
 ****************************************************************************************
 */
void gapc_sdt_init(uint8_t init_type);

/**
 ****************************************************************************************
 * @brief Convert attribute index to attribute handle
 *
 * @param[in] att_idx       Attribute index
 *
 * @return Attribute handle
 ****************************************************************************************
 */
uint16_t gapc_svc_hdl_get(uint8_t att_idx);

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

#endif // (BLE_GAPC)
/// @} GAPC_INT

#endif /* _GAPC_INT_H_ */
