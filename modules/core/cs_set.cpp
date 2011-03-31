/* ChanServ core functions
 *
 * (C) 2003-2011 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 */

/*************************************************************************/

#include "module.h"

class CommandCSSet : public Command
{
	typedef std::map<Anope::string, Command *, std::less<ci::string> > subcommand_map;
	subcommand_map subcommands;

 public:
	CommandCSSet() : Command("SET", 2, 3)
	{
		this->SetDesc(_("Set channel options and information"));
	}

	~CommandCSSet()
	{
		this->subcommands.clear();
	}

	CommandReturn Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		User *u = source.u;

		if (readonly)
		{
			source.Reply(_(CHAN_SET_DISABLED));
			return MOD_CONT;
		}
		if (!check_access(u, cs_findchan(params[0]), CA_SET))
		{
			source.Reply(_(ACCESS_DENIED));
			return MOD_CONT;
		}

		// XXX Remove after 1.9.4 release
		if (params[1].equals_ci("MLOCK"))
		{
			source.Reply(_(CHAN_SET_MLOCK_DEPRECATED), Config->s_ChanServ.c_str());
			return MOD_CONT;
		}

		Command *c = this->FindCommand(params[1]);

		if (c)
		{
			ChannelInfo *ci = source.ci;
			Anope::string cmdparams = ci->name;
			for (std::vector<Anope::string>::const_iterator it = params.begin() + 2, it_end = params.end(); it != it_end; ++it)
				cmdparams += " " + *it;
			mod_run_cmd(ChanServ, u, NULL, c, params[1], cmdparams);
		}
		else
		{
			source.Reply(_(NICK_SET_UNKNOWN_OPTION), Config->UseStrictPrivMsgString.c_str(), params[1].c_str());
			source.Reply(_(MORE_INFO), Config->UseStrictPrivMsgString.c_str(), Config->s_ChanServ.c_str(), "SET");
		}

		return MOD_CONT;
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{
		if (subcommand.empty())
		{
			source.Reply(_("Syntax: \002SET \037channel\037 \037option\037 \037parameters\037\002\n"
					" \n"
					"Allows the channel founder to set various channel options\n"
					"and other information.\n"
					" \n"
					"Available options:"));
			for (subcommand_map::iterator it = this->subcommands.begin(), it_end = this->subcommands.end(); it != it_end; ++it)
				it->second->OnServHelp(source);
			source.Reply(_("Type \002%s%s HELP SET \037option\037\002 for more information on a\n"
					"particular option."), Config->UseStrictPrivMsgString.c_str(), ChanServ->nick.c_str());
			return true;
		}
		else
		{
			Command *c = this->FindCommand(subcommand);

			if (c)
				return c->OnHelp(source, subcommand);
		}

		return false;
	}

	void OnSyntaxError(CommandSource &source, const Anope::string &subcommand)
	{
		SyntaxError(source, "SET", _(CHAN_SET_SYNTAX));
	}

	bool AddSubcommand(Module *creator, Command *c)
	{
		c->module = creator;
		c->service = this->service;
		return this->subcommands.insert(std::make_pair(c->name, c)).second;
	}

	bool DelSubcommand(Command *c)
	{
		return this->subcommands.erase(c->name);
	}

	Command *FindCommand(const Anope::string &subcommand)
	{
		subcommand_map::const_iterator it = this->subcommands.find(subcommand);

		if (it != this->subcommands.end())
			return it->second;

		return NULL;
	}
};

class CSSet : public Module
{
	CommandCSSet commandcsset;

 public:
	CSSet(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Anope");
		this->SetType(CORE);

		this->AddCommand(ChanServ, &commandcsset);
	}
};

MODULE_INIT(CSSet)
