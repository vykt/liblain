set environment CK_FORK=no

# print vm_area at a node
define parean
	if $argc != 1
		printf "Use: parean <*area_node>\n"
	else
		p *((mc_vm_area *) ($arg0->data))
	end
end

# hex print vm_area at a node
define pxarean
	if $argc != 1
		printf "Use: parean <*area_node>\n"
	else
		p/x *((mc_vm_area *) ($arg0->data))
	end
end

# print vm_obj at a node
define pobjn
	if $argc != 1
		printf "Use: pobjn <*obj_node>\n"
	else
		p *((mc_vm_obj *) ($arg0->data))
	end
end

# hex print vm_obj at a node
define pxobjn
	if $argc != 1
		printf "Use: pobjn <*obj_node>\n"
	else
		p/x *((mc_vm_obj *) ($arg0->data))
	end
end

# print a node at the specified node
define pnode
	if $argc != 1
		printf "Use: pnode <*obj_area_node>\n"
	else
		p *(*((cm_lst_node **) ($arg0->data)))
	end
end

# print the area at a node at the specified node:
define pnodea
	if $argc != 1
		printf "Use: pnodea <*obj_area_node>\n"
	else
		p *((mc_vm_area *) ((*((cm_lst_node **) ($arg0->data)))->data))
	end
end

# hex print the are aat a node at the specified node:
define pxnodea
	if $argc != 1
		printf "Use: pnodea <*obj_area_node>\n"
	else
		p/x *((mc_vm_area *) ((*((cm_lst_node **) ($arg0->data)))->data))
	end
end

tb test__map_forward_unmapped_obj_last_vm_areas_fn
run
