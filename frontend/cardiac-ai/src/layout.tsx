import { Box } from "@mui/material";
import React from "react";

const Layout = ({ children }: { children: React.ReactNode }) => {
	return <Box sx={{ width: "100%", margin: "4rem 0 0" }}>{children}</Box>;
};

export default Layout;
