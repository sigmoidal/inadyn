/* Plugin for TZO (Acquired by DYN)
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2006       Steve Horbachuk
 * Copyright (C) 2010-2014  Joachim Nilsson <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "plugin.h"

/* tzo doesnt encode password */
#define TZO_UPDATE_IP_REQUEST						\
	"GET %s?"							\
	"tzoname=%s&"							\
	"email=%s&"							\
	"tzokey=%s&"							\
	"ipaddress=%s&"							\
	"system=tzodns&"						\
	"info=1 "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"User-Agent: " AGENT_NAME " " SUPPORT_ADDR "\r\n\r\n"

static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t plugin = {
	.name         = "default@tzo.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = "echo.tzo.com",
	.checkip_url  = "/",
	.checkip_ssl  = DDNS_CHECKIP_SSL_NOT_SUPPORTED,

	.server_name  = "rh.tzo.com",
	.server_url   = "/webclient/tzoperl.html"
};

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	return snprintf(ctx->request_buf, ctx->request_buflen,
			TZO_UPDATE_IP_REQUEST,
			info->server_url,
			alias->name,
			info->creds.username,
			info->creds.password,
			alias->address,
			info->server_name.name);
}

/* TZO specific response validator. */
static int response(http_trans_t *trans, ddns_info_t *UNUSED(info), ddns_alias_t *UNUSED(alias))
{
	int code = -1;

	DO(http_status_valid(trans->status));

	sscanf(trans->p_rsp_body, "%4d ", &code);
	switch (code) {
	case 200:
	case 304:
		return 0;
	case 414:
	case 500:
		return RC_DYNDNS_RSP_RETRY_LATER;
	default:
		break;
	}

	return RC_DYNDNS_RSP_NOTOK;
}

PLUGIN_INIT(plugin_init)
{
	plugin_register(&plugin);
}

PLUGIN_EXIT(plugin_exit)
{
	plugin_unregister(&plugin);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
