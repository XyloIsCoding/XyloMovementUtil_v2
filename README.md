# XyloMovementUtil_v2
This plugin aims at making life easier while extending the CharacterMovementComponent.
In particular it improves the extensibility of its client prediciton / reconciliation system, so it can be used for generic purposes (e.g. integrating the weapon system in the movement simulation, so that recoil is predicted).
It also aims to provides a modularized extension of cmc, so that movement modes, layered moves (like crouching, proning and sprinting), and jump profiles (how character respondes to jumping: double jump, glide jump, jetpack) are easier to implement and are added as external objects
