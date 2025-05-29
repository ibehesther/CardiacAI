import { Box, Button, Input, Typography } from "@mui/material";
import React from "react";

const Login = () => {
	return (
		<Box
			sx={{
				width: "100vw",
				height: "calc(100vh - 4rem)",
				display: "flex",
				flexDirection: "column",
				justifyContent: "center",
				alignItems: "center",
				background: 'rgba(249, 250, 255, 1) url("/images/heart_rate.png") no-repeat fixed bottom',
        backgroundSize: 'contain'
			}}
		>
			<Typography
				sx={{
					fontSize: { xs: "1.875rem", sm: "3rem", md: "3.5rem" },
					fontWeight: 900,
					color: "rgba(18, 56, 144, 1)",
					lineHeight: { xs: "2rem", sm: "3rem", md: "4rem" },
				}}
			>
				Get Started
			</Typography>
			<Box
				sx={{
					marginTop: "10rem",
					display: "flex",
					flexDirection: "column",
					gap: "3rem",
					width: { xs: "80%", md: "60%", lg: "50%" },
				}}
			>
				<Input
					placeholder="Input device ID"
					sx={{
						border: "2px solid rgba(0, 0, 0, 0.4)",
						padding: "1rem",
						fontSize: { xs: "1.25rem", sm: "1.5rem" },
						borderRadius: "1rem",
						":focus": {
							border: "2px solid rgba(61, 119, 242, 1)",
						},
						"::after": {
							border: 0,
						},
						"::before": {
							borderBottom: "0 !important",
						},
					}}
				/>
				<Button
					variant="contained"
					sx={{
						backgroundColor: "rgba(61, 119, 242, 1)",
						padding: { xs: "1rem", sm: "1rem 1.5rem" },
						borderRadius: "1.25rem",
						fontSize: { xs: "1.25rem", sm: "1.5rem" },
					}}
          onClick={() => window.open('/analysis', '_self')}
				>
					Get Started
				</Button>
				<Button
					variant="text"
					sx={{
						color: "rgba(61, 119, 242, 1)",
						fontSize: { xs: "1rem", sm: "1.25rem" },
						textTransform: "capitalize",
					}}
				>
					Forgotten device ID?
				</Button>
			</Box>
		</Box>
	);
};

export default Login;
