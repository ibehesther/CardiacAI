import { Box, Button, Typography } from "@mui/material";
import React from "react";

const Header = () => {
	const isUserAuthenticated = false;
	return (
		<header>
			<Box
				sx={{
					position: "fixed",
					top: 0,
					left: 0,
					right: 0,
					backgroundColor: "rgba(249, 250, 255, 1)",
					padding: "1.5rem",
					display: "flex",
					justifyContent: "space-between",
					alignItems: "center",
					height: "4rem",
					zIndex: 100
				}}
			>
				<Typography
					variant="h1"
					component={"a"}
					href="/"
					sx={{
						textDecoration: "none",
						color: "#000",
						fontSize: "1.5rem",
						fontWeight: 600,
					}}
				>
					CardiacAI
				</Typography>
				<Box
					sx={{
						display: "flex",
						flexDirection: "row",
						alignItems: "center",
						gap: { xs: "1.5rem", sm: "3rem" },
					}}
				>
					<Typography
						component={"a"}
						href="/about"
						sx={{
							textDecoration: "none",
							color: "#000",
							":hover": {
								color: "rgba(61, 119, 242, 1)",
							},
							fontSize: { xs: "1rem", md: "1.2rem" },
						}}
					>
						About
					</Typography>
					<Button
						variant="contained"
						onClick={() => window.open("/login", "_self")}
						sx={{
							backgroundColor: "rgba(61, 119, 242, 1)",
							padding: { xs: "1rem", sm: "1rem 1.5rem" },
							borderRadius: "1.25rem",
							fontSize: { xs: "0.75rem", sm: "1rem" },
						}}
					>
						{isUserAuthenticated ? "Log out" : "Get Started"}
					</Button>
				</Box>
			</Box>
		</header>
	);
};
export default Header;
